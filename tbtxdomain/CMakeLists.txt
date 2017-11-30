set(TBTXDOMAIN "tbtxdomain")
set(TBTXDOMAIN_RULES "${RULES_PREFIX}-${TBTXDOMAIN}.rules")

configure_file("${TBTXDOMAIN}.in" ${TBTXDOMAIN} @ONLY)
configure_file("${TBTXDOMAIN}.rules.in" ${TBTXDOMAIN_RULES} @ONLY)

install(PROGRAMS            "${CMAKE_CURRENT_BINARY_DIR}/${TBTXDOMAIN}"
        DESTINATION          ${UDEV_BIN_DIR})
install(FILES               "${CMAKE_CURRENT_BINARY_DIR}/${TBTXDOMAIN_RULES}"
        DESTINATION          ${UDEV_RULES_DIR})
