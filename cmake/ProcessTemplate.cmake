# IN_FILE - input file
# OUT_FILE - output file
# NAME - name to replace

if (NOT EXISTS "${IN_FILE}")
    message (FATAL_ERROR "Input file \"${IN_FILE}\" doesn't exist!")
endif ()

message (STATUS "Processing template \"${IN_FILE}\", updating name to \"${NAME}\"")

file (READ ${IN_FILE} text)
string (REPLACE "##NAME##" "${NAME}" text "${text}")
file (WRITE "${OUT_FILE}" "${text}")

