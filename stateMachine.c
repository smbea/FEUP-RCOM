//State Machine SET
//WIP
struct tram{
	int firstFlag = 0x7E;
	int messageType;
	char header;
}


int main()
{
	int flag1;
	char c;
	struct tram alpha;
	int tempByte = 0;
	
	while(1)
	{
		do //check flag
		{ 
			tempByte = readByte();
		}while(tempByte != alpha.firstFlag)
	
		do //check message type
		{ 
			tempByte = readByte();
		}while(tempByte != 3 && tempByte != 1 && tempByte != alpha.firstFlag)
	
		if(tempByte == 3)
			alpha.messageType = 0;
		else if(tempByte == 1)
			alpha.messageType = 1;
		else
			continue;
	
		do
		{
			tempByte = readByte();
		}while(tempByte != 0x02 && tempByte != 0x0B && tempByte != 0x07 && /*!*/ tempByte != 0x05 && tempByte != 0x01 && tempByte != alpha.firstFlag)
		
		switch(tempByte)
		{
			case 0x02:
				alpha.header = 'S';
				break;
			case 0x0B:
				alpha.header = 'D';
				break;
			case 0x07:
				alpha.header = 'U';
				break;
			case 0x05:
				alpha.header = 'R';
				break;
			case 0x01:
				alpha.header = 'J';
				break;
			default:
				continue;
		}
		break;
	}	
}
