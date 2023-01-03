mkdir -p ../bin
cd ../src && gcc *.c
cd ../src/ && mv a.out ../bin/viper 

../bin/viper ../examples/class_example_3.viper