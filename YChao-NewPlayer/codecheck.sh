find -type f -name *.cpp -o -name *.c -o -name *.h | egrep -v 'test|vcproj|cppunit|pthread|taglib' | xargs cpplint.py --counting=detailed --linelength=160 --filter=-,+whitespace/comma,+whitespace/braces,+whitespace/empty_conditional_body,+whitespace/empty_loop_body,+whitespace/forcolon,+whitespace/semicolon,+whitespace/parens,+whitespace/newline,+whitespace/operators 2> cpplint-report.xml

cppcheck -j 4 --enable=all --xml-version=2 osal/inc osal/src media/common media/audiofx media/codec/*/*/TT*Plugin.* media/info media/player -imedia/player/tests -imedia/player/win32test 2> cppcheck-report.xml
