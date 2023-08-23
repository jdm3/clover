/*
Clover - a simple, single-header library for parsing command line arguments.
         Does the world need another one? No, but here you go!

Copyright (c) 2023 Jefferson Montgomery

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

// To use char-based arguments, define CLOVER_USE_WCHAR_T=0
// before including opts.h
#ifndef CLOVER_USE_WCHAR_T
#define CLOVER_USE_WCHAR_T 1
#endif

/*
EXAMPLE USAGE
=============

    CommandLineOptions opts;

    // AddOption() has the following parameters:
    //     value: a pointer to a variable where the parsed results will be sored,
    //     name: the text to match for this option,
    //     valueDesc: an optional description for the option's value,
    //     description: an optional description for the option,
    //     includeInUsage: whether this option is reported during PrintUsage().
    opts.AddOption(...);
    opts.AddOption(...);
    ...

    int errorArgIndex = 0;
    switch (opts.Parse(argc, argv, &errorArgIndex)) {
    case CommandLineOptions_Ok: break;
    case CommandLineOptions_HelpRequested:
        opts.PrintUsage();
        return 0;
    case CommandLineOptions_ErrorArgumentExpectingValue:
        fprintf(stderr, "error: command line argument expecting value: %ls.\n", argv[errorArgIndex]);
        opts.PrintUsage();
        return 1;
    case CommandLineOptions_ErrorArgumentValueInvalid:
        fprintf(stderr, "error: invalid command line argument value: %ls.\n", argv[errorArgIndex]);
        opts.PrintUsage();
        return 1;
    case CommandLineOptions_ErrorUnrecognisedArgument:
        fprintf(stderr, "error: unrecognised command line argument: %ls.\n", argv[errorArgIndex]);
        opts.PrintUsage();
        return 1;
    }


MATCHING COMMAND LINE ARGUMENTS
===============================

An option will match with a command line argument "-NAME=...", "--NAME=...", or
"/NAME=...", ignoring case, with the following exceptions:

    - bool options do not use the "=..." part.

    - CharT* options with valueDesc==nullptr match any command line argument
      not starting with "-", "--", or "/".  If there are multiple such options,
      they match command line arguments in the same order they were added to
      the CommandLineOptions instance.

PRINTING USAGE
==============

Usage is printed in the following format:
    usage: EXE_NAME [options] ARGUMENTS
    options:
        --NAME=VALUEDESC DESCRIPTION
        ...

ARGUMENTS is a space-separated list of CharT* options that have valueDesc==nullptr (if any).

The lines after "options:" correspond to AddOption() and AddUsageNewLine()
calls, and are printed in the same order that AddOption/AddUsageNewLine() were
called.  Exceptions to the general option format above include:

    - bool options do not include the "=VALUEDESC" part.

    - CharT* options with a valueDesc==nullptr print "NAME DESCRIPTION".
*/

enum CommandLineOptionsResult {
    CommandLineOptions_Ok,
    CommandLineOptions_HelpRequested,
    CommandLineOptions_ErrorArgumentExpectingValue,
    CommandLineOptions_ErrorArgumentValueInvalid,
    CommandLineOptions_ErrorUnrecognisedArgument,
};

class CommandLineOptions {
public:
    #if CLOVER_USE_WCHAR_T
    using CharT = wchar_t;
    #else
    using CharT = char;
    #endif

    void AddOption(bool*     value, CharT const* name,                         CharT const* description, bool includeInUsage=true);
    void AddOption(uint32_t* value, CharT const* name, CharT const* valueDesc, CharT const* description, bool includeInUsage=true);
    void AddOption(CharT**   value, CharT const* name, CharT const* valueDesc, CharT const* description, bool includeInUsage=true);

    void AddUsageNewLine();

    uint32_t GetOptionCount(bool includeNewlines=false) const;

    // Print usage (see above). Option descriptions are wrapped at any
    // whitespace exceeding the line's targetWidth.
    void PrintUsage(FILE* fp=stderr, int targetWidth=100) const;

    // Parses the command line arguments.
    //
    // If errorArgIndex!=nullptr and the returned
    // result!=CommandLineOptions_Ok, then that result was caused by the
    // argument at argv[*errorArgIndex].
    CommandLineOptionsResult Parse(int argc, CharT** argv, int* errorArgIndex);

    // After Parse() has been called, returns whether a particular option
    // matched an argument in the command line.
    bool WasFound(CharT const* name) const;

private:
    struct Option {
        CharT const* name_;
        CharT const* valueDesc_;
        CharT const* description_;
        void* value_;
        enum { NEWLINE, ARG, BOOL, UINT32, STRING, } type_;
        bool includeInUsage_;
        bool found_;
    };

    std::vector<Option> options_;
};

#if CLOVER_USE_WCHAR_T
#define CLOVER_MAKESTR(_A)          L ## _A
#define CLOVER_stricmp(_A, _B)      _wcsicmp(_A, _B) == 0
#define CLOVER_strnicmp(_A, _B, _C) _wcsnicmp(_A, _B, _C) == 0
#define CLOVER_strlen(_A)           wcslen(_A)
#define CLOVER_strtoul(_A, _B, _C)  wcstoul(_A, _B, _C)
#define CLOVER_fprintf(_A, ...)     fwprintf(fp, L ## _A, __VA_ARGS__)
#else
#define CLOVER_MAKESTR(_A)          _A
#define CLOVER_stricmp(_A, _B)      _stricmp(_A, _B) == 0
#define CLOVER_strnicmp(_A, _B, _C) _strnicmp(_A, _B, _C) == 0
#define CLOVER_strlen(_A)           strlen(_A)
#define CLOVER_strtoul(_A, _B, _C)  strtoul(_A, _B, _C)
#define CLOVER_fprintf(_A, ...)     fprintf(fp, _A, __VA_ARGS__)
#endif

void CommandLineOptions::AddOption(bool* value, CharT const* name, CharT const* description, bool includeInUsage)
{
    options_.emplace_back(Option{ name, nullptr, description, (void*) value, Option::BOOL, includeInUsage, false });
}

void CommandLineOptions::AddOption(uint32_t* value, CharT const* name, CharT const* valueDesc, CharT const* description, bool includeInUsage)
{
    options_.emplace_back(Option{ name, valueDesc, description, (void*) value, Option::UINT32, includeInUsage, false });
}

void CommandLineOptions::AddOption(CharT** value, CharT const* name, CharT const* valueDesc, CharT const* description, bool includeInUsage)
{
    options_.emplace_back(Option{ name, valueDesc, description, (void*) value, valueDesc == nullptr ? Option::ARG : Option::STRING, includeInUsage, false });
}

void CommandLineOptions::AddUsageNewLine()
{
    options_.emplace_back(Option{ nullptr, nullptr, nullptr, nullptr, Option::NEWLINE, true, false });
}

void CommandLineOptions::PrintUsage(FILE* fp, int targetWidth) const
{
    // Scan options to determine option width, etc.
    size_t colWidth = 0;
    bool hasOptions = false;
    for (auto const& opt : options_) {
        if (opt.type_ != Option::NEWLINE && opt.type_ != Option::ARG) {
            hasOptions = true;
            colWidth = std::max(colWidth, (opt.name_      == nullptr ? 0 : CLOVER_strlen(opt.name_)) +
                                          (opt.valueDesc_ == nullptr ? 0 : CLOVER_strlen(opt.valueDesc_) + 1));
        }
    }
    colWidth += 8;

    // usage: exe [options] arg arg ...
    {
        CharT path[MAX_PATH];
        #if CLOVER_USE_WCHAR_T
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        std::wstring filename(path);
        #else
        GetModuleFileNameA(nullptr, path, MAX_PATH);
        std::string filename(path);
        #endif

        filename.erase(0, filename.find_last_of(CLOVER_MAKESTR("/\\")) + 1);

        auto n = filename.size();
        if (n > 4 && CLOVER_stricmp(filename.c_str() + n - 4, CLOVER_MAKESTR(".exe"))) {
            filename.resize(n - 4);
        }

        CLOVER_fprintf("usage: %s", filename.c_str());
    }
    if (hasOptions) {
        CLOVER_fprintf(" [options]");
    }
    for (auto const& opt : options_) {
        if (opt.type_ == Option::ARG) {
            CLOVER_fprintf(" %s", opt.name_);
        }
    }
    CLOVER_fprintf("\n");

    // options:
    //     --name=value    desc...
    if (hasOptions) {
        CLOVER_fprintf("options:\n");
        for (auto const& opt : options_) {
            if (opt.includeInUsage_) {
                int x = 0;
                if (opt.name_ != nullptr) {
                    x += CLOVER_fprintf("    --%s", opt.name_);
                }
                if (opt.valueDesc_ != nullptr) {
                    x += CLOVER_fprintf("=%s", opt.valueDesc_);
                }
                if (opt.description_ != nullptr) {
                    x += CLOVER_fprintf(" ");
                    for (; x < colWidth; ++x) {
                        CLOVER_fprintf(" ");
                    }
                    for (auto p = opt.description_; *p; ++p) {
                        if (x > targetWidth && *p == ' ') {
                            x = (int) colWidth;
                            CLOVER_fprintf("\n%*s", x, CLOVER_MAKESTR(""));
                        } else {
                            x += CLOVER_fprintf("%c", *p);
                        }
                    }
                }
                CLOVER_fprintf("\n");
            }
        }
    }
}

CommandLineOptionsResult CommandLineOptions::Parse(int argc, CharT** argv, int* errorArgIndex)
{
    int argIndex = 1;

    auto Error = [&argIndex, errorArgIndex](CommandLineOptionsResult result) {
        if (errorArgIndex != nullptr) {
            *errorArgIndex = argIndex;
        }
        return result;
    };

    for ( ; argIndex < argc; ++argIndex) {
        auto arg = argv[argIndex];

        bool hasPrefix = true;
        if (*arg == '/') {
            ++arg;
        } else if (*arg == '-') {
            ++arg;
            if (*arg == '-') {
                ++arg;
            }
        } else {
            hasPrefix = false;
        }

        if (hasPrefix && (CLOVER_stricmp(arg, CLOVER_MAKESTR("?")) ||
                          CLOVER_stricmp(arg, CLOVER_MAKESTR("h")) ||
                          CLOVER_stricmp(arg, CLOVER_MAKESTR("help")))) {
            return Error(CommandLineOptions_HelpRequested);
        }

        bool found = false;
        for (auto& opt : options_) {
            if (opt.type_ == Option::ARG) {
                if (!hasPrefix && opt.found_ == false) {
                    *((CharT**) opt.value_) = arg;
                    opt.found_ = true;
                    found = true;
                    break;
                }
            }

            else if (opt.type_ == Option::BOOL) {
                if (hasPrefix && CLOVER_stricmp(arg, opt.name_)) {
                    *((bool*) opt.value_) = true;
                    opt.found_ = true;
                    found = true;
                    break;
                }
            }

            else if (opt.type_ != Option::NEWLINE) {
                if (hasPrefix) {
                    auto n = CLOVER_strlen(opt.name_);
                    if (CLOVER_strnicmp(arg, opt.name_, n)) {
                        if (arg[n] == '\0') {
                            return Error(CommandLineOptions_ErrorArgumentExpectingValue);
                        }
                        if (arg[n] == '=') {
                            arg += n + 1;
                            switch (opt.type_) {
                            case Option::UINT32: {
                                uint32_t* p = (uint32_t*) opt.value_;
                                CharT* end = nullptr;
                                *p = CLOVER_strtoul(arg, &end, 0);
                                if (*end != '\0' || (*p == 0 && (end == arg || errno != 0))) {
                                    return Error(CommandLineOptions_ErrorArgumentValueInvalid);
                                }
                            }   break;
                            case Option::STRING:
                                *((CharT**) opt.value_) = arg;
                                break;
                            }
                            opt.found_ = true;
                            found = true;
                            break;
                        }
                    }
                }
            }
        }

        if (!found) {
            return Error(CommandLineOptions_ErrorUnrecognisedArgument);
        }
    }

    return CommandLineOptions_Ok;
}

uint32_t CommandLineOptions::GetOptionCount(bool includeNewlines) const
{
    uint32_t count = (uint32_t) options_.size();
    if (!includeNewlines) {
        for (auto const& opt : options_) {
            if (opt.type_ == Option::NEWLINE) {
                count -= 1;
            }
        }
    }
    return count;
}

bool CommandLineOptions::WasFound(CharT const* name) const
{
    for (auto const& opt : options_) {
        if (CLOVER_stricmp(name, opt.name_)) {
            return opt.found_;
        }
    }
    return false;
}

#undef CLOVER_MAKESTR
#undef CLOVER_stricmp
#undef CLOVER_strnicmp
#undef CLOVER_strlen
