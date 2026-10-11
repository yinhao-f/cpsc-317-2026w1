output := ./out
test_output := ./test/out
src_file := ./sources
test_file := ./tests
src_extensions := *.java
run_class := ca.ubc.cs.cs317.dnslookup.DNSLookupCUI
test_package := ca.ubc.cs.cs317.dnslookup

all: compile_src compile_test

run:
	java -cp ${output} ${run_class}

run_test:
	java -jar ./test/junit-platform-console-standalone-1.9.0.jar -cp ${output}:${test_output} --select-package ${test_package}

clean:
	rm -rf ${output}/*

compile_test: find_tests
	javac -cp ./test/junit-platform-console-standalone-1.9.0.jar:${output} -d ${test_output} @${test_file}

compile_src: find_sources
	javac -d ${output} @${src_file}

find_sources:
	find ./src/ ${sources} -name "${src_extensions}" > ${src_file}

find_tests:
	find ./test/ ${tests} -name "${src_extensions}" > ${test_file}
