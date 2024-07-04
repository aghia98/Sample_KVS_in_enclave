cd gRPC_module
source env.sh
make

cd ..

cd HRW_hashing_module
make

cd ..

cd Token_module
source env.sh
make

cd ..

cd SS_no_gmp_module
make