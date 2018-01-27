#include "Motor.h"
#include "Interface.h"

// Status Byte�� �д´�
#define READ_STATUS(X, Y)   ((Y) = MOTOR_CMD(X))
// Status Byte�� Busy Bit�� üũ�Ͽ� ���µǸ� ������ Ż���Ѵ�.
#define	CHECK_BUSY(X, Y)	while(((Y) = MOTOR_CMD(X)) & 0x01)


// Status Byte�� ������ ����
unsigned char status;

// Function  : LM629���� �����͸� �д´�.
// Parameter :
//      motor - ������ ����
// Return    :
//      data  - ���� ������
unsigned char ReadData(unsigned char motor)
{
    unsigned char data;

    // Status Byte�� Busy Bit üũ
    CHECK_BUSY(motor, status);
    // ������ �Է�
    data = MOTOR_DATA(motor);

	CHECK_BUSY(motor, status);
    // �Էµ� ������ ����
    return data;
}    


// Function  : Ŀ�ǵ带 LM629�� ����.
// Parameter :
//      motor - ������ ����
//      cmd   - ������ Ŀ�ǵ�
// Return    : ����

void WriteCommand(unsigned char motor, unsigned char cmd)
{
    // Status Byte�� Busy Bit üũ
    CHECK_BUSY(motor, status);
    // Ŀ�ǵ� ���
    MOTOR_CMD(motor) = cmd;

	CHECK_BUSY(motor, status);
}


// Function  : 2 ����Ʈ �����͸� LM629�� ����.
// Parameter :
//      motor - ������ ����
//      data  - �� ������
// Return    : ����
void WriteDataTwoByte(unsigned char motor, int data)
{
    // Status Byte�� Busy Bit üũ
    CHECK_BUSY(motor, status);
    // 2 ����Ʈ ������ ���
    MOTOR_DATA(motor) = (uint8_t)((data >> 8)&0xFF);
	CHECK_BUSY(motor, status);
    MOTOR_DATA(motor) = (uint8_t)(data&0xFF);
	CHECK_BUSY(motor, status);
}


// Function  : 4 ����Ʈ �����͸� LM629�� ����.
// Parameter :
//      motor - ������ ����
//      data  - �� ������
// Return    : ����
void WriteDataFourByte(unsigned char motor, long data)
{
    // 4 ����Ʈ�� �����͸� �ѷ� ����� WriteDataTwoByte�Լ��� �ι� ȣ��
    WriteDataTwoByte(motor, (int16_t)(data >> 16));
    WriteDataTwoByte(motor, (int16_t)(data));
}


// Function  : ���� �ʱ�ȭ
// Parameter : ����
// Return    : ����
void Motor_init(void)
{	
    // Motor1 ���� ����
    SetGain(MOTOR0, 7, 3, 1);
    // Motor2 ���� ����
    SetGain(MOTOR1, 7, 3, 1);
    // Motor3 ���� ����
    SetGain(MOTOR2, 7, 3, 1);

    SetAcceleration(MOTOR0, 0x3FFFFFFF);
    SetAcceleration(MOTOR1, 0x3FFFFFFF);
    SetAcceleration(MOTOR2, 0x3FFFFFFF);
}


// Function  : LM629�� �����Ѵ�.
// Parameter :
//      motor - ������ ����
// Return    : ����
void Motor_Reset(uint8_t motor)
{
    // RESET Ŀ�ǵ� ���
    WriteCommand(motor, RESET);
    // ������ �Ϸ�� ������ �ּ� 1.5ms�̻� ���
    _delay_ms(2);
	// Morot1 ���ͷ�Ʈ ����ũ ����
    WriteCommand(motor, RSTI);
    WriteDataTwoByte(motor, 0x0000);
}

// Function  : ���Ͱ� KP, KI, KD ���� �����Ѵ�.
// Parameter :
//      motor - ������ ����
//      kp    - P��
//      ki    - I��
//      kd    - D��
// Return    : ����
void SetGain(uint8_t motor, int16_t kp, int16_t ki, int16_t kd)
{
    unsigned char LD_kp=0, LD_ki=0, LD_kd=0;

	if(kp)LD_kp=1;
	else LD_kp=0;
	if(ki)LD_ki=1;
	else LD_ki=0;
	if(kd)LD_kd=1;
	else LD_kd=0;
	// LFIL Ŀ�ǵ� ���
    WriteCommand(motor, LFIL);
    // ��Ʈ�� ���� ���, sampling interval�� 341us ����
    WriteDataTwoByte(motor, (0x0000) | (LD_kp << 3) | (LD_ki << 2) | (LD_kd << 1)  );
    ////////////////////////////
	// P�� ���
	if(kp)WriteDataTwoByte(motor, kp);
    // I�� ���
    if(ki)WriteDataTwoByte(motor, ki);
    // D�� ���
    if(kd)WriteDataTwoByte(motor, kd);
    // ���� ������Ʈ
    WriteCommand(motor, UDF);
}


// Function  : ���ӵ����� �����Ѵ�.
// Parameter :
//      motor - ������ ����
//      acc   - ���ӵ���
// Return    : ����
void SetAcceleration(uint8_t motor, int32_t acc)
{
    // LTRJ Ŀ�ǵ� ���
    WriteCommand(motor, LTRJ);
    // ��Ʈ�� ���� ���
    WriteDataTwoByte(motor, (1 << ACC_LOAD));
    // ���ӵ��� ���
    WriteDataFourByte(motor, acc);
}


// Function  : �ӵ����� �����Ѵ�.
// Parameter :
//      motor    - ������ ����
//      velocity - �ӵ� ����  (0 ~ 1,073,741,823) ���� �Է½� ���������� ȸ��
// Return    : ����
void SetVelocity(uint8_t motor, int32_t velocity)
{
	unsigned char dir=1;

	if(velocity<0){
		velocity*=(-1);
		dir=0;
	}
    // LTRJ Ŀ�ǵ� ���
    WriteCommand(motor, LTRJ);        
    // ��Ʈ�� ���� ���        
    WriteDataTwoByte(motor,  (dir << FORWARD_DIR) | (1 << VELOCITY_MODE) | (1 << VELOCITY_LOAD));
    // �ӵ��� ���
    WriteDataFourByte(motor, velocity);
}

// Function  : ���ӵ����� �����Ѵ�.
// Parameter :
//      motor - ������ ����
//      mode  - ��� ����    (0 -> Absolute Position, 1 -> Relative Position)
//		acc   - ���ӵ� ����  (0 ~ 1,073,741,823)
//   velocity - �ӵ� ����    (0 ~ 1,073,741,823)
//   position - ������ġ���� (-1,073,741,824 ~ 1,073,741,823)counts
// Return    : ����
void SetPosition(uint8_t motor, uint8_t mode, int32_t acc,int32_t velocity,int32_t position)
{
    // LTRJ Ŀ�ǵ� ���
    WriteCommand(motor, LTRJ);
    // ��Ʈ�� ���� ���
    // mode : 0 - Absolute Position, 1 - Relative Position
    // Position Mode�� ����
    
	WriteDataTwoByte(motor, (1 << ACC_LOAD) | (1 << VELOCITY_LOAD) | (1 << POSITION_LOAD) | (mode << POSITION_RELATIVE) );//WriteDataTwoByte(motor, 0x2A|mode);
    // ��ġ�� ���
	WriteDataFourByte(motor, acc);    
	WriteDataFourByte(motor, velocity);
    WriteDataFourByte(motor, position);
}

// Function  : LM629 H/W Reset. ���� ���� �ΰ��� �ݵ�� �����ؾ��Ѵ�.
// Parameter : ����
// Return    : ����
void LM629_HW_Reset(void){
	unsigned char status[3]={0, }, i=0, step=0;
	
	DDRA=0x00;

	PORTC=0x00;
	DDRC=0xFF;

	PORTG=0x00;
	DDRG=0x03;

	while(1){
		if(step==0){
			while(1){
				PORTC |= 0x05; //Hw Rst Hi(decoder = Low)
				_delay_ms(3);
				PORTC &= ~0x05; //Hw Rst Low(decoder = Hi)
				_delay_ms(2);

				PORTG &= ~0x02; //RD low
				PORTC &= ~0x08; //PS low
		
				PORTC &=~0x07;	//cs motor0
				status[0]=PINA; //Read status motor0
				
				PORTC |=0x01;	//cs motor1
				status[1]=PINA; //Read status motor0
				
				PORTC = (PORTC&0xF8)|0x02; 	//cs motor2
				status[2]=PINA;  //Read status motor0

				if( (status[0]==0xC4||status[0]==0x84) && 
					(status[1]==0xC4||status[1]==0x84) && 
					(status[2]==0xC4||status[2]==0x84) ){
						step=1;
						break;
				}
				else{
					PORTC=0x00;
					lcd_display_str(0,0,"Error. Restart");
				}
			}
		}
		
		if(step==1){
			// SRE : �ܺ� ������ �޸� ���� Ȱ��ȭ
			MCUCR = (1 << SRE) | (0 << SRW10);
		    // SRL(0) : Upper Sector�θ� ��� 0x1100 ~ 0xFFFF(Upper)
		    // SRW11:SRW10(1:0) - Read/Write ��ȣ�� 2���� Wait
			XMCRA = (0 << SRL2) | (0 << SRL1) | (0 << SRL0) | (1 << SRW11);
		    //XMBK : Bus Keeper Enable, XMM(0x4) - A11 ~ A8������ �ܺ� �޸� ��Ʈ�� ���
			XMCRB = (1 << XMBK) | (1 << XMM2) | (0 << XMM1) | (0 << XMM0);
			while(1){
				for(i=0;i<3;++i){
		        	WriteCommand(i, RSTI);
					WriteDataTwoByte(i, 0x0000);
					READ_STATUS(i, status[i]);
				}
				if( (status[0]==0xC0||status[0]==0x80) && 
					(status[1]==0xC0||status[1]==0x80) && 
					(status[2]==0xC0||status[2]==0x80) ){
					step=2;
					break;
				}
				else{	//�ٽ� ó������ LM629�ʱ�ȭ
					step=0;	
					// SRE : �ܺ� ������ �޸� ���� ��Ȱ��ȭ
					MCUCR &= ~(1 << SRE) & ~(1 << SRW10);
					PORTC=0x00;
					lcd_display_str(0,0,"Error. Restart");
					break;
				}
			}
		}
		if(step==2){
			lcd_display_str(0,0,"Initialize success.");
			for(i=0;i<3;++i){
				PORTB|=0x07;
				_delay_ms(150);
				PORTB&=~0x07;
				_delay_ms(150);
			}
			lcd_clear_screen();
			break;
		}
	}
}
	