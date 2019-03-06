#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "InternalNode.h"

#include "tls-with1.3.h"


using std::cout;
using std::endl;
using std::string;


int main(int argc , char *argv[]) {

    string hex = "0x"
            "160301012c01000128030340c70e243001b96d8c63687738696432d3e6f949"
			"107aabad8450cdffd6a266e4000092c030c02cc028c024c014c00a00a3009f"
			"006b006a0039003800880087c032c02ec02ac026c00fc005009d003d003500"
			"84c02fc02bc027c023c013c00900a2009e0067004000330032009a00990045"
			"0044c031c02dc029c025c00ec004009c003c002f00960041c011c007c00cc0"
			"0200050004c012c00800160013c00dc003000a001500120009001400110008"
			"0006000300ff0100006d000b000403000102000a00340032000e000d001900"
			"0b000c00180009000a00160017000800060007001400150004000500120013"
			"000100020003000f0010001100230000000d0020001e060106020603050105"
			"020503040104020403030103020303020102020203000f000101";

	VectorBuffer buf;

    buf.appendFromString(hex);


    TVector_MainType node;

    node.dissector().dissectFromBuffer(buf);

    node.print();

    
    cout << endl << endl;


    DataUnit* testnode = node.getByPath("**/ClientHello_extensions%/_V/~2");

    testnode->getSibling("~-2", true)->print();
    testnode->getSibling("~-1", true)->print();
    testnode->getSibling("~0", true)->print();
    testnode->getSibling("~1", true)->print();
    testnode->getSibling("~2", true)->print();


	return EXIT_SUCCESS;
}



