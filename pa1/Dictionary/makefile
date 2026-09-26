output := ./out
src_file := ./sources
src_extensions := *.java
run_class := ca.ubc.cs317.dict.ui.DictionaryMain

all: compile_src compile_test

run:
	java -cp ${output} ${run_class}

clean:
	rm -rf ${output}/*

compile_test:
	

compile_src: find_sources
	javac -d ${output} @${src_file}

find_sources:
	find ./src/ ${sources} -name "${src_extensions}" > ${src_file}

