#Workaround for CPack RPM generator to get a behavior similar to
#CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST available in CPack 2.8.12 (for supporting
#RedHat, which uses v2.8.11)
set(CPACK_RPM_SPEC_MORE_DEFINE "%define ignore \#")
set(CPACK_RPM_USER_FILELIST "%ignore /etc"
                            "%ignore /etc/init.d"
                            "%ignore /usr"
                            "%ignore /usr/share"
                            "%ignore /usr/share/doc"
                            "%ignore /usr/bin"
                            "%ignore /usr/lib"
                            "%ignore /usr/lib64"
                            "%ignore /usr/include"
                            )

