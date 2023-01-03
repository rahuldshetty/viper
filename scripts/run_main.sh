mkdir -p ../bin
cd ../src && gcc *.c
cd ../src/ && mv a.out ../bin/viper 

if [ $# -eq 1 ]
then
    ../bin/viper $1
else 
    ../bin/viper
fi