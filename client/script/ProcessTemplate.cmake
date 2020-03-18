# IN_FILE - input file
# OUT_FILE - output file
# NAME - name to replace
# CLIENT_OPTS -client options

if (NOT EXISTS "${IN_FILE}")
    message (FATAL_ERROR "Input file \"${IN_FILE}\" doesn't exist!")
endif ()

file (READ ${IN_FILE} text_1)
string (REPLACE "##NAME##" "${NAME}" text_2 "${text_1}")
string (REPLACE "##CLIENT_OPTS##" "${CLIENT_OPTS}" text_3 "${text_2}")

file (WRITE "${OUT_FILE}" "${text_3}")

