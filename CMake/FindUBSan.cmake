# The MIT License (MIT)
#
# Copyright (c)
#   2013 Matthew Arsenault
#   2015-2016 RWTH Aachen University, Federal Republic of Germany
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

option(PIPE_SANITIZE_UNDEFINED
    "Enable UndefinedBehaviorSanitizer for sanitized targets." Off)

set(FLAG_CANDIDATES
    "-g -fsanitize=undefined -fno-sanitize=alignment"
)


include(SanitizeHelpers)

if (PIPE_SANITIZE_UNDEFINED)
    pipe_sanitizer_check_compiler_flags("${FLAG_CANDIDATES}"
        "UndefinedBehaviorSanitizer" "UBSan")
endif ()

function (pipe_add_sanitize_undefined TARGET)
    if (NOT PIPE_SANITIZE_UNDEFINED)
        return()
    endif ()

    pipe_sanitizer_add_flags(${TARGET} "UndefinedBehaviorSanitizer" "UBSan")
endfunction ()
