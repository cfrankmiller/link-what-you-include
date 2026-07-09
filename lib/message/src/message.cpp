// Copyright (c) 2025 Environmental Systems Research Institute, Inc.
// SPDX-License-Identifier: Apache-2.0

#include <message/message.hpp>

#include <iostream>
#include <string>
#include <string_view>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace message
{
namespace
{
struct Output_options
{
  bool color{false};
  Message_level message_level{Message_level::normal};
};

Output_options g_options{}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

#ifdef _WIN32
bool enable_virtual_terminal_processing(HANDLE handle)
{
  if (handle == INVALID_HANDLE_VALUE || handle == nullptr)
  {
    return false;
  }

  DWORD mode = 0;
  if (!GetConsoleMode(handle, &mode))
  {
    return false;
  }

  if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0)
  {
    return true;
  }

  return SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
}
#endif

bool is_smart_terminal()
{
  // TODO: we currently only need a smart terminal on STDOUT. If that changes, also check
  //       STD_ERROR_HANDLE/STDERR_FILENO.
#ifdef _WIN32
  return enable_virtual_terminal_processing(GetStdHandle(STD_OUTPUT_HANDLE));
#else
  return isatty(STDOUT_FILENO);
#endif
}

Message_level message_level()
{
  return g_options.message_level;
}
} // namespace

void configure(Color_output color, Message_level message_level)
{
  switch (color)
  {
    case Color_output::never:
    {
      g_options.color = false;
      break;
    }
    case Color_output::always:
    {
      g_options.color = true;
      break;
    }
    case Color_output::automatic:
    {
      g_options.color = is_smart_terminal();
      break;
    }
  }

  g_options.message_level = message_level;
}

std::string paint(std::string_view text, Style style)
{
  if (!g_options.color || style == Style::plain)
  {
    return std::string{text};
  }

  const char* code = "";
  switch (style)
  {
    case Style::plain:
      return std::string{text};
    case Style::heading:
      code = "1;36";
      break;
    case Style::success:
      code = "1;32";
      break;
    case Style::warning:
      code = "1;33";
      break;
    case Style::error:
      code = "1;31";
      break;
    case Style::note:
      code = "1;34";
      break;
  }

  return std::string{"\x1b["} + code + "m" + std::string{text} + "\x1b[0m";
}

bool verbose_enabled()
{
  return message_level() >= Message_level::verbose;
}

void print(std::string_view text, Style style)
{
  std::cout << paint(text, style) << '\n';
}

void blank_line()
{
  std::cout << '\n';
}

void heading(std::string_view text)
{
  print(text, Style::heading);
}

void status(std::string_view label, std::string_view text, Style style)
{
  std::cout << paint(label, style) << ": " << text << '\n';
}

void info(std::string_view text)
{
  status("info", text);
}

void error(std::string_view text)
{
  status("error", text, Style::error);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void error_block(std::string_view title, std::string_view details)
{
  status("error", title, Style::error);
  if (!details.empty())
  {
    print(details);
  }
}

void note(std::string_view text)
{
  status("note", text, Style::note);
}

void warning(std::string_view text)
{
  status("warning", text, Style::warning);
}

bool debug_enabled()
{
  return message_level() >= Message_level::debug;
}

void debug(std::string_view text)
{
  if (debug_enabled())
  {
    status("debug", text, Style::warning);
  }
}
} // namespace message
