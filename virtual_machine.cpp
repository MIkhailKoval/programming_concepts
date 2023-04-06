#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>

static const int REGISTERS_COUNT = 8; 

/*
    COMMANDS:
      0X1 => ADD
      0X2 => SUB
      0X3 => MUL
      0X4 => DIV

      0X5 => TODO
      0X6 => TODO
      0X7 => TODO
      0X8 => TODO
      0X9 => TODO
      0X10 => TODO
      0X11 => TODO

      0X12 => PRINT
      
*/
class VirtualMachine {
    public:
        VirtualMachine(std::ifstream& in): in_(in), registers_(REGISTERS_COUNT, 0), data_() {}
        void ProcessProgram() {
            for (int i = 0; i < registers_.size(); ++i) {
                registers_[i] = ReadInt();
                std::cout << "register " << i + 1 << " is " << registers_[i] << std::endl;
            }
            
            // read data
            int data_start = bytes_read;
            char c{'0'};
            while (bytes_read < GetRegisterData(0)) {
                in_.get(c);
                data_.push_back(c);
                bytes_read++;
            }

            // read and execute program
            while (true) {
                in_.get(c);
                ProcessCommand(c);
                return;
            }

        }
        int& GetRegisterData(int register_num) {
            return registers_[register_num];
        }
    private:
        std::vector<int> registers_;
        std::vector<char> data_;
        int bytes_read = 0;
        std::ifstream& in_;

        int ReadInt() {
            int x = 0;
            for (int j = 0; j < 4; ++j) {
                char c;
                in_.get(c);
                x |= (uint8_t)c << (24 - j * 8);
                bytes_read ++;
            }
            return x;
        }
        
        void ProcessCommand(char c) {
            if (c == 12) {
                return PrintNumber();
            }
        }

        void PrintNumber() {
            int x = ReadInt();
            for (int i = x - data_start; data_[i] != '\0'; i++) {
                std::cout << data_[i];
            }
            std::cout << std::endl;
        }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "usage ./virtual_machine program_name.bin" << std::endl; 
        return 1;
    }
    std::string filename = argv[1];
    std::ifstream in;
    in.open(filename);
    VirtualMachine virtual_machine{in};
    virtual_machine.ProcessProgram();
}
