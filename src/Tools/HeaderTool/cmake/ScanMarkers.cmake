# Usage: cmake -DCAND=<candidates.cmake> -DOUT=<manifest.cmake> -P ScanMarkers.cmake
include("${CAND}")                      # defines SCYTHE_CANDIDATE_HEADERS
set(_marked "")
foreach(_h IN LISTS SCYTHE_CANDIDATE_HEADERS)
    file(READ "${_h}" _contents)
    if(_contents MATCHES "SCYTHE_CLASS\\(")
        list(APPEND _marked "${_h}")
    endif()
endforeach()

set(_text "set(SCYTHE_MARKED_HEADERS\n")
foreach(_m IN LISTS _marked)
    string(APPEND _text "  \"${_m}\"\n")
endforeach()
string(APPEND _text ")\n")

# Write-if-changed: stable modification time => no spurious reconfigure.
set(_old "")
if(EXISTS "${OUT}")
    file(READ "${OUT}" _old)
endif()
if(NOT _old STREQUAL _text)
    file(WRITE "${OUT}" "${_text}")
endif()