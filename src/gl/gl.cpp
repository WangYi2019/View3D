
#include "gl.hpp"

#include <iostream>
#include <cassert>
#include <vector>
#include <memory>

namespace gl
{

// ==========================================================================
// misc

void check_log_error_1 (const char* file, int lineno)
{
  GLenum err = glGetError ();
  if (err == GL_NO_ERROR)
    return;

  const char* errstr = "";
  switch (err)
  {
    case GL_INVALID_ENUM: errstr = "GL_INVALID_ENUM"; break;
    case GL_INVALID_VALUE: errstr = "GL_INVALID_VALUE"; break;
    case GL_INVALID_OPERATION: errstr = "GL_INVALID_OPERATION"; break;
    case GL_INVALID_FRAMEBUFFER_OPERATION: errstr = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
    case GL_OUT_OF_MEMORY: errstr = "GL_OUT_OF_MEMORY"; break;
    default: errstr = ""; break;
  }

  std::cerr << "glGetError (" << file << ':' << lineno << "): " << errstr
            << std::endl;
}

frame_stats cur_frame_stats;

// ==========================================================================
// buffer

unsigned int buffer::g_cur_vtx_bound = 0;
unsigned int buffer::g_cur_idx_bound = 0;


// ==========================================================================
// shader

GLuint shader::g_cur_program = INVALID_PROGRAM_ID;

shader::~shader (void)
{
  if (m_vertex_shader)
    glDeleteShader (m_vertex_shader);
  if (m_fragment_shader)
    glDeleteShader (m_fragment_shader);
  if (m_program)
    glDeleteProgram (m_program);

  if (g_cur_program == m_program && g_cur_program != INVALID_PROGRAM_ID)
  {
    glUseProgram (INVALID_PROGRAM_ID);
    g_cur_program = INVALID_PROGRAM_ID;
  }
}

void shader::activate (void)
{
  if (g_cur_program != INVALID_PROGRAM_ID && g_cur_program == m_program)
    return;

  check_try_compile ();

  glUseProgram (m_program);
  g_cur_program = m_program;
  cur_frame_stats.shader_select_count += 1;
}

void shader::check_try_compile (void)
{
  if (m_tried_compile)
    return;

  m_tried_compile = true;

  // on GL_VENDOR: Humper, GL_RENDERER:Chromium GL_SHADER_COMPILER returns
  // false, even though a shader compiler is available.  thus, don't do the
  // check.  just try compiling it.
  /*
  {
    GLboolean compiler_available = false;
    glGetBooleanv (GL_SHADER_COMPILER, &compiler_available);
    if (!compiler_available)
    {
      std::cerr << "glGetBooleanv GL_SHADER_COMPILER NG" << std::endl;
      return;
    }
  }*/

  std::string vertex_src;
  vertex_src.reserve (1024);

  std::string fragment_src;
  fragment_src.reserve (1024);

  for (std::vector<parameter*>::const_iterator i = m_uniforms.begin (),
       iend = m_uniforms.end (); i != iend; ++i)
  {
    std::string pstr = (*i)->make_decl ();
    vertex_src += pstr;
    vertex_src += '\n';
    fragment_src += pstr;
    fragment_src += '\n';
  }

  for (std::vector<parameter*>::const_iterator i = m_attributes.begin (),
       iend = m_attributes.end (); i != iend; ++i)
  {
    std::string pstr = (*i)->make_decl ();
    vertex_src += pstr;
    vertex_src += '\n';
  }

  vertex_src += vertex_shader_text_str ();
  const char* vertex_src_cstr = vertex_src.c_str ();
  m_vertex_shader = glCreateShader (GL_VERTEX_SHADER);
  glShaderSource (m_vertex_shader, 1, &vertex_src_cstr, nullptr);
  glCompileShader (m_vertex_shader);

  {
    GLint compile_stat;
    glGetShaderiv (m_vertex_shader, GL_COMPILE_STATUS, &compile_stat);
    if (!compile_stat)
    {
      GLint loglen = 0;
      glGetShaderiv (m_vertex_shader, GL_INFO_LOG_LENGTH, &loglen);
      char* str = (char*)alloca ((loglen + 32) * sizeof (char));
      glGetShaderInfoLog (m_vertex_shader, loglen, nullptr, str);
      std::cerr << "vertex shader compile NG:\n"
                << vertex_src << '\n'
                << (const char*)str << std::endl;
      glDeleteShader (m_vertex_shader);
      m_vertex_shader = INVALID_PROGRAM_ID;
    }
  }

  fragment_src += fragment_shader_text_str ();
  const char* fragment_src_cstr = fragment_src.c_str ();
  m_fragment_shader = glCreateShader (GL_FRAGMENT_SHADER);
  glShaderSource (m_fragment_shader, 1, &fragment_src_cstr, nullptr);
  glCompileShader (m_fragment_shader);

  {
    GLint compile_stat;
    glGetShaderiv (m_fragment_shader, GL_COMPILE_STATUS, &compile_stat);
    if (!compile_stat)
    {
      GLint loglen = 0;
      glGetShaderiv (m_fragment_shader, GL_INFO_LOG_LENGTH, &loglen);
      char* str = (char*)alloca ((loglen + 32) * sizeof (char));
      glGetShaderInfoLog (m_fragment_shader, loglen, nullptr, str);
      std::cerr << "fragment shader compile NG:\n"
                << fragment_src << '\n'
                << (const char*)str << std::endl;

      glDeleteShader (m_fragment_shader);
      m_fragment_shader = INVALID_PROGRAM_ID;
    }
  }
 
  if (m_fragment_shader == INVALID_PROGRAM_ID
      || m_vertex_shader == INVALID_PROGRAM_ID)
    return;

  m_program = glCreateProgram ();
  glAttachShader (m_program, m_vertex_shader);
  glAttachShader (m_program, m_fragment_shader);

  glLinkProgram (m_program);

  GLint link_stat = false;
  glGetProgramiv (m_program, GL_LINK_STATUS, &link_stat);
  if (!link_stat)
  {
    std::cerr << "glLinkProgram NG" << std::endl;
    glDeleteProgram (m_program);
    m_program = INVALID_PROGRAM_ID;
    return;
  }

  glValidateProgram (m_program);

  GLint validate_stat = false;
  glGetProgramiv (m_program, GL_VALIDATE_STATUS, &validate_stat);
  if (!validate_stat)
  {
    std::cerr << "glValidateProgram NG" << std::endl;
    glDeleteProgram (m_program);
    m_program = INVALID_PROGRAM_ID;
    return;
  }

  for (std::vector<parameter*>::iterator i = m_uniforms.begin (),
       iend = m_uniforms.end (); i != iend; ++i)
  {
    (*i)->m_location = glGetUniformLocation (m_program, (*i)->name ());
//    std::cerr << "uniform " << (*i)->name ()
//              << " location = " << (*i)->location () << std::endl;
  }

  for (std::vector<parameter*>::iterator i = m_attributes.begin (),
       iend = m_attributes.end (); i != iend; ++i)
  {
    (*i)->m_location = glGetAttribLocation (m_program, (*i)->name ());
//    std::cerr << "attribute " << (*i)->name ()
//              << " location = " << (*i)->location () << std::endl;
  }
}

} // namespace gl
