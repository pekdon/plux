What is PLUX?!
==============

PLUX is a system testing tool inspired by LUX written in C++ that aims
to be easy to use for testing applications.

PLUX is **not** intended as a replacement or substitute for unit- and
integration-testing but is instead a complement for testing the final
application.

Differences compared to LUX
===========================

PLUX is line based
------------------

PLUX operate on a line-by-line basis and as such **^** and **$**
regular expression markers actually mean the start and end of a line.

PLUX skip output when entering command
--------------------------------------

PLUX treat the lines that send output to the shell as matched so
there is no risk for accidentaly matching output that was entered
on the command line.

PLUX match error pattern continously
------------------------------------

PLUX will match the error pattern on complete lines as they are
received without the use of any match command.
