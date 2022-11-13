#!/bin/bash

# change these per each homework
#	link to tests:
testsurl="https://webcourse.cs.technion.ac.il/fc159753hw_236360_202201/hw/WCFiles/hw1-tests.zip"

tmpdir="selfcheck_tmp"
if [ ! -f "submission.zip" ]
	then
		zip submission.zip hw1.cpp scanner.lex tokens.hpp
		
fi

if [ ! -f "submission.zip" ]
	then
		echo "Submission zip file not found!"

		exit
fi

rm -rf "$tmpdir" &> /dev/null
if [ -d "$tmpdir" ]
	then
		echo "Cannot clear tmp directory. Please delete '"$tmpdir"' manually and try again"
		exit
fi
mkdir "$tmpdir" &> /dev/null

unzip "submission.zip" -d "$tmpdir" &> /dev/null
if [[ $? != 0 ]] 
	then
		echo "Unable to unzip submission file!"
		exit
fi

cd "$tmpdir"
if [ ! -f scanner.lex ]
	then
		echo "File scanner.lex not found!"
		exit
fi

flex scanner.lex &> /dev/null
if [[ $? != 0 ]] 
	then
		echo "Cannot build submission!"
		exit
fi
g++ -std=c++17 lex.yy.c hw1.cpp -o hw1.out &> /dev/null
if [[ $? != 0 ]] 
	then
		echo "Cannot build submission!"
		exit
fi
if [ ! -f hw1.out ]
	then
		echo "Cannot build submission!"
		exit
fi

RED="\e[31m"
GREEN="\e[32m"
ENDCOLOR="\e[0m"

#	number of tests:
numtests=57
#	command to execute test:
command="./hw1.out < ../tests/input/t\$i.in >& ../tests/output/t\$i.out"
i="1"
failed=false
rm -rf ../tests/output &> /dev/null
mkdir ../tests/output
while [ $i -le $numtests ]
	do
		eval "$command"
		diff ../tests/output/t$i.out ../tests/expected/t$i.out &> /dev/null
		if [[ $? != 0 ]] 
			then
				echo -e "${RED}FAILED${ENDCOLOR} test #"$i"!"
				failed=true
		fi
		i=$((i+1))
done

if $failed ; then
	cd - &> /dev/null
	rm -rf "$tmpdir"
    exit
fi

cd - &> /dev/null
rm -rf "$tmpdir"

echo -e "All tests ${GREEN}PASSED${ENDCOLOR}! Ok to submit :)"
exit
