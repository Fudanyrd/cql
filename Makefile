compile = g++ -std=c++11 -c -Wall
expr = expr.h expr.cpp
string_util = string_util.h string_util.cpp
schema = schema.h schema.cpp
tuple = tuple.h tuple.cpp
type = type.h type.cpp
table = table.h table.cpp

# object generating
string_util.o: $(string_util)
	$(compile) string_util.cpp
schema.o: $(schema)
	$(compile) schema.cpp 
type.o: $(type)
	$(compile) type.cpp
tuple.o: $(tuple)
	$(compile) tuple.cpp
table.o: $(table)
	$(compile) table.cpp
expr.o: $(expr)
	$(compile) expr.cpp

# unit test
stringUtilTest: string_util.o string_util_test.cpp
	$(compile) string_util_test.cpp
	g++ -o util string_util.o string_util_test.o

schemaTest: schema.o string_util.o schema_test.cpp
	$(compile) schema_test.cpp
	g++ -o schema string_util.o schema.o schema_test.o

tupleTest: schema.o string_util.o type.o tuple.o tuple_test.cpp
	$(compile) tuple_test.cpp
	g++ -o tuple string_util.o schema.o type.o tuple.o tuple_test.o

tableTest: schema.o string_util.o type.o tuple.o table.o table_test.cpp
	$(compile) table_test.cpp
	g++ -o table schema.o string_util.o type.o tuple.o table.o table_test.o

# cleaning workspace
clean:
	rm *.o