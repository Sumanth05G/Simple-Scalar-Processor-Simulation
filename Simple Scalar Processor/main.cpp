#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdint>
using namespace std;

class fetchbuffer{
    public:
        uint8_t InsNo;
};

class decodebuffer{
    public:
        uint8_t  InsNo;
        uint16_t IR;
};

class execbuffer{
    public:
        uint8_t InsNo;
        uint8_t operation;
        uint8_t destreg;
        int8_t signedimm;
        uint8_t unsignimm;
        int8_t A;
        int8_t B;
        uint16_t IR;
};

class memorybuffer{
    public:
        uint8_t oper;
        uint8_t InsNo;
        uint8_t destreg;
        int8_t AluOut;
        int8_t B;
        uint16_t IR;
};

class writebackcuffer{
    public:
        uint8_t InsNo;
        int8_t AluOut;
        int8_t LMD;
        uint8_t destreg;
        uint8_t oper;
        uint16_t IR;
};


int main(){
    int8_t DCache[256];
    uint8_t ICache[256];
    int8_t R[16];

    int Total_ins = 0;
    int Arithimetic_ins = 0;
    int Logical_ins = 0;
    int Shift_ins = 0;
    int Memory_ins = 0;
    int Load_imm = 0;
    int Control_ins = 0;
    int Halt_ins = 0;

    int Total_stalls = 0;
    int Data_stalls = 0;
    int Control_stalls = 0;

    ifstream InsCac_rd ("input/ICache.txt");
    ifstream DatCac_rd ("input/DCache.txt");
    ifstream RF_rd ("input/RF.txt");
    for(int i = 0;i<256;i++){
        string temp;
        InsCac_rd >> temp;
        ICache[i] = static_cast<uint8_t> (stoi(temp, nullptr, 16));
    }
    for(int i = 0;i<256;i++){
        string temp;
        DatCac_rd >> temp;
        DCache[i] = static_cast <int8_t> (stoi(temp, nullptr, 16));
    }
    for(int i = 0;i<16;i++){
        string temp;
        RF_rd >> temp;
        R[i] = static_cast <int8_t>(stoi(temp, nullptr, 16));
    }
    
    int clockcycles = 0;

    fetchbuffer fb;
    decodebuffer db;
    execbuffer eb;
    memorybuffer mb;
    writebackcuffer wb;

    int pc = 0;
    bool fetch = true;
    bool decode = false;
    bool exec = false;
    bool memo = false;
    bool writeb = false;
    int causeforstall = -1;
    int dangerous[16] = {0};
    for(int i = 0;i<16;i++){
        dangerous[i] = 0;
    }



    while(1){
        
        clockcycles++;
        if(causeforstall!=3) {fetch = true;}

        if(writeb)
        {   
            if(wb.oper == 15){
                break;
            }
            if((wb.oper>=0) && (wb.oper<=10)){
                R[wb.destreg] = wb.AluOut;
                dangerous[wb.destreg]--;
            }
            if(wb.oper == 11){
                R[wb.destreg] = wb.LMD;
                dangerous[wb.destreg]--;
            }
            
        }else
        {

        }


        if(memo)
        {   
            writeb  = true;
            wb.IR = mb.IR;
            wb.destreg = mb.destreg;
            wb.AluOut = mb.AluOut;
            wb.oper = mb.oper;
            if(mb.oper == 11){
                wb.LMD = DCache[mb.AluOut];
                wb.destreg = mb.destreg;
            }
            if(mb.oper == 12){
                DCache[mb.AluOut] = mb.B;
            }
        }else
        {
            writeb = false;
        }


        if(exec)
        {   
            memo = true;
            mb.IR = eb.IR;
            mb.destreg = eb.destreg;
            mb.oper = eb.operation;
            uint8_t op = eb.operation;
            int8_t Aluout;
            Total_ins++;

            if(op==0){
                Aluout = eb.A + eb.B;
                mb.AluOut = Aluout;
                Arithimetic_ins++;
            }
            if(op==1){
                Aluout = eb.A - eb.B;
                mb.AluOut = Aluout;
                Arithimetic_ins++;
            }
            if(op==2){
                Aluout = eb.A * eb.B;
                mb.AluOut = Aluout;
                Arithimetic_ins++;
            }
            if(op==3){
                Aluout = eb.A + 1;
                mb.AluOut = Aluout;
                Arithimetic_ins++;
            }
            if(op==4){
                Aluout = eb.A & eb.B;
                mb.AluOut = Aluout;
                Logical_ins++;
            }
            if(op==5){
                Aluout = eb.A | eb.B;
                mb.AluOut = Aluout;
                Logical_ins++;
            }
            if(op==6){
                Aluout = eb.A ^ eb.B;
                mb.AluOut = Aluout;
                Logical_ins++;
            }
            if(op==7){
                Aluout = ~ eb.A ;
                mb.AluOut = Aluout;
                Logical_ins++;
            }
            if(op==8){
                Aluout = eb.A << eb.unsignimm;
                mb.AluOut = Aluout;
                Shift_ins++;
            }
            if(op==9){
                uint8_t temp = static_cast<uint8_t>(eb.A);
                temp = temp >> eb.unsignimm;
                Aluout = static_cast<int8_t> (temp);
                mb.AluOut = Aluout;
                Shift_ins++;
            }
            if(op==10){
                Aluout = eb.signedimm;
                mb.AluOut = Aluout;
                Load_imm++;
            }
            if(op==11){
                Aluout = eb.A + eb.signedimm;
                mb.AluOut = Aluout;
                Memory_ins++;
            }
            if(op==12){
                Aluout = eb.A + eb.signedimm;
                mb.AluOut = Aluout;
                Memory_ins++;
                mb.B = eb.B;
            }
            if(op==13){
                pc = pc + 2*eb.signedimm;
                fetch = false;
                Control_ins++;
            }
            if(op==14){
                if(eb.A == 0){
                    pc = pc + 2*eb.signedimm;
                    fetch = false;
                    Control_ins++;
                }else{
                    fetch = false;
                    Control_ins++;
                }
            }
            if(op==15){
                decode = false;
                Halt_ins++;
            }

        }
        else
        {
            memo = false;
        }


        if(decode)
        {   
            exec = true;

            eb.IR = db.IR;

            uint16_t ins = db.IR;
            uint8_t oper = ins >> 12;

            eb.InsNo = db.InsNo;
            eb.operation = oper;
            uint8_t destreg;
            uint8_t srcreg1;
            uint8_t srcreg2;
            int8_t signedimm;
            uint8_t unsignimm;
            int8_t A;
            int8_t B;

            if(oper == 0 || oper == 1 || oper == 2 || oper == 4 || oper == 5 || oper == 6){
                destreg = ins >> 8;
                destreg = destreg & 0b00001111;
                srcreg1 = ins >> 4;
                srcreg1 = srcreg1 & 0b00001111;
                srcreg2 = ins;
                srcreg2 = srcreg2 & 0b00001111;
                if(dangerous[srcreg1]!=0||dangerous[srcreg2]!=0){
                    causeforstall = 2;
                    exec = false;
                    fetch = false;
                }else{
                    dangerous[destreg] ++;
                    A = R[srcreg1];
                    B = R[srcreg2];
                    eb.A = A;
                    eb.B = B;
                    eb.destreg =  destreg;
                }
            }
            if(oper == 3){
                destreg = ins >> 8;
                destreg = destreg & 0b00001111;
                srcreg1 = destreg;
                if(dangerous[srcreg1]!=0){
                    causeforstall = 2;
                    exec = false;
                    fetch = false;
                }
                else{
                    dangerous[destreg]++;
                    A = R[srcreg1];
                    eb.A = A;
                    eb.destreg = destreg;
                }
            }
            if(oper == 7){
                destreg = ins >> 8;
                destreg = destreg & 0b00001111;
                srcreg1 = ins >> 4;
                srcreg1 = srcreg1 & 0b00001111;
                if(dangerous[srcreg1]!=0){
                    causeforstall = 2;
                    exec = false;
                    fetch = false;
                }else{
                    dangerous[destreg] ++;
                    A = R[srcreg1];
                    eb.A = A;
                    eb.destreg = destreg;
                }
            }
            if(oper == 8 || oper == 9){
                destreg = ins >> 8;
                destreg = destreg & 0b00001111;
                srcreg1 = ins >> 4;
                srcreg1 = srcreg1 & 0b00001111;
                unsignimm = ins;
                unsignimm = unsignimm & 0b00001111;
                if(dangerous[srcreg1] != 0){
                    causeforstall = 2;
                    exec = false;
                    fetch = false;
                }else{
                    dangerous[destreg] ++;
                    eb.unsignimm = unsignimm;
                    eb.A = R[srcreg1];
                    eb.destreg = destreg;
                }
            }
            if(oper == 10){
                destreg = ins >> 8;
                destreg = destreg & 0b00001111;
                uint8_t imm = ins;
                signedimm = static_cast<int8_t> (imm);
                dangerous[destreg] ++;
                eb.destreg = destreg;
                eb.signedimm = signedimm;
            }
            if(oper == 11){
                destreg = ins >> 8;
                destreg = destreg & 0b00001111;
                srcreg1 = ins >> 4;
                srcreg1 = srcreg1 & 0b00001111;
                uint8_t imm = ins;
                imm = imm & 0b00001111;
                if((imm & 0b00001000)!=0){ imm = imm | 0b11110000;}
                signedimm = static_cast<int8_t> (imm);
                if(dangerous[srcreg1]!=0){
                    causeforstall = 2;
                    exec = false;
                    fetch = false;
                }else{
                    dangerous[destreg] ++;
                    eb.A = R[srcreg1];
                    eb.destreg = destreg;
                    eb.signedimm = signedimm;
                }
            }
            if(oper == 12){
                srcreg1 = ins >> 4;
                srcreg1 = srcreg1 & 0b00001111;
                srcreg2 = ins >> 8;
                srcreg2 = srcreg2 & 0b00001111;
                uint8_t imm = ins;
                imm = imm & 0b00001111;
                if((imm & 0b00001000) != 0){ imm = imm | 0b11110000;}
                signedimm = static_cast<int8_t> (imm);
                // cout<<(int)(signedimm)<<'\n';
                if(dangerous[srcreg1]!=0 || dangerous[srcreg2]!=0){
                    causeforstall = 2;
                    exec = false;
                    fetch = false;
                }else{
                    eb.A = R[srcreg1];
                    eb.B = R[srcreg2];
                    eb.signedimm = signedimm;
                }
            }
            if(oper == 13){
                uint8_t t1 = ins >> 4;
                t1 = t1 & 0b11111111;
                signedimm = static_cast<int8_t> (t1);
                eb.signedimm = signedimm;
                fetch = false;
                causeforstall = 1;
            }
            if(oper == 14){
                srcreg1 = ins >> 8;
                srcreg1 = srcreg1 & 0b00001111;
                uint8_t t1 = ins;
                signedimm = static_cast<int8_t> (t1);
                if(dangerous[srcreg1]!=0){
                    causeforstall = 2;
                    exec = false;
                    fetch = false;
                }else{
                    eb.A = R[srcreg1];
                    eb.signedimm = signedimm;
                    fetch = false;
                    causeforstall = 1;
                }
            }
            if(oper == 15){
                fetch = false;
                causeforstall = 3;
            }
        }else
        {
            exec = false;
        }


        if(fetch)
        {   
            uint16_t ins = 0x0000;
            uint16_t fir = ICache[pc];
            uint16_t sec = ICache[pc+1];
            fir = fir << 8;
            ins = fir | sec;

            db.InsNo = pc;
            db.IR = ins;
            pc = pc + 2;
            decode = true;
        }else
        {
            decode = false;
            if(causeforstall==2){decode = true;}
            if(causeforstall==1){Control_stalls++;Total_stalls++;}
            if(causeforstall==2){Data_stalls++;Total_stalls++;}
        }
    }
    cout<<"Hi\n";
    ofstream oDcache ("output/DCache.txt");
    ofstream out ("output/Output.txt");

    for(int i = 0;i<256;i++){
        int8_t dc = DCache[i];
        uint8_t temp = dc >> 4;
        temp = temp & 0b00001111;
        if(temp>=10){
            oDcache<<(char)('a'+temp-10);
        }else{
            oDcache<<(char)('0'+temp);
        }
        temp = dc & 0b00001111;
        if(temp>=10){
            oDcache<<(char)('a'+temp-10)<<'\n';
        }else{
            oDcache<<(char)('0'+temp)<<'\n';
        }
    }

    double cpi = (double)(clockcycles)/(double)(Total_ins);

    out << "Total number of instructions executed        : "<<Total_ins<<'\n';
    out << "Number of instructions in each class\n";
    out << "Arithmetic instructions                      : "<<Arithimetic_ins<<'\n';
    out << "Logical instructions                         : "<<Logical_ins<<'\n';
    out << "Shift instructions                           : "<<Shift_ins<<'\n';
    out << "Memory instructions                          : "<<Memory_ins<<'\n';
    out << "Load immediate instructions                  : "<<Load_imm<<'\n';
    out << "Control instructions                         : "<<Control_ins<<'\n';
    out << "Halt instructions                            : "<<Halt_ins<<'\n';
    out << "Cycles Per Instruction                       : "<<cpi<<'\n';
    out << "Total number of stalls                       : "<<Total_stalls<<'\n';
    out << "Data stalls (RAW)                            : "<<Data_stalls<<'\n';
    out << "Control stalls                               : "<<Control_stalls<<'\n';

}
