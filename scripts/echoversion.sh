grep AC_INIT ../configure.ac | sed -e "s/AC_INIT[(][[]blitwizard[]], [[]//g" | sed -e "s/[]])//g"
