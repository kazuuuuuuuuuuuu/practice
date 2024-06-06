.proto -> .h .cc 
protoc -I=./ --cpp_out=./ bike.proto

.cpp .cc -> .out
 1916  g++ --std=c++11  case2.cpp bike.pb.cc -lprotobuf -o test -lpthread
