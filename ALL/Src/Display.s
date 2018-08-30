;/******************** (C) COPYRIGHT 2017 e-Design Co.,Ltd. ********************
; File Name : Display.s
; Version   : For DS212 Ver 1.0 With STM32F303VC                  Author : bure
;******************************************************************************/
    RSEG VIEW:CODE(3)
    PRESERVE8

    IMPORT    __SetPosi
    IMPORT    __SendPixels

BACKGROUND    = 0x0000         ;// ������ɫ
GRID_COLOR    = 0x7BEF         ;// ���������ߵ���ɫ
WR            = 0x020          ;// Gpio Pin 5
P_HID         = 0x01
L_HID         = 0x02
D_HID         = 0x01
W_HID         = 0x04
                               ;//    0 ~  409: LCD Col Buffer 
P_TAB         =  410           ;//  410 ~  499: ParamTab 
A_BUF         =  500           ;//  500 ~  799: Wave Track#1 Buffer
B_BUF         =  800           ;//  800 ~ 1099: Wave Track#2_Buffer 
C_BUF         = 1100           ;// 1100 ~ 1399: Wave Track#3_Buffer 
P_BUF         = 1400           ;// 1400 ~ 8191: Pop Buffer


CH_A          = P_TAB+2*0      ;// Wave Track#1 Flag
CH_B          = P_TAB+2*1      ;// Wave Track#2 Flag
CH_C          = P_TAB+2*2      ;// Wave Track#3 Flag
CCHA          = P_TAB+2*18     ;// Wave Track#1 Color
CCHB          = P_TAB+2*19     ;// Wave Track#2 Color
CCHC          = P_TAB+2*20     ;// Wave Track#3 Color
M_X0          = P_TAB+2*30     ;// ������ʾ������ʼλ�� X0
M_Y0          = P_TAB+2*31     ;// ������ʾ������ʼλ�� Y0
M_WX          = P_TAB+2*32     ;// ������ʾ����ˮƽ��� WX
POPF          = P_TAB+2*33     ;// Pop Flag
PXx1          = P_TAB+2*34     ;// Pop X Position
PWx1          = P_TAB+2*35     ;// Pop Width
PYx2          = P_TAB+2*36     ;// Pop Y Position *2
PHx2          = P_TAB+2*37     ;// Pop Hight *2

    EXPORT  Align00
    EXPORT  Align01
    EXPORT  Align02
    EXPORT  Align03
    EXPORT  Align04
    EXPORT  Align05
    EXPORT  Align06
    EXPORT  Align07
    EXPORT  Align08
    EXPORT  Align09
    EXPORT  Align10
    EXPORT  Align11

;//=============================================================================
;//                  View ���ڲ�����ʾ��ػ�������ӳ���
;//=============================================================================
;// void __DrawWindow(u32 VRAM_Addr)
;//=============================================================================
    EXPORT  __DrawWindow
    NOP.W
    NOP.W
__DrawWindow
    PUSH    {R4-R12,LR}
    NOP.W                      ;// R0: VRAM ��ʼָ��
    LDRH    R1,  [R0, #M_WX]   ;// �������ڵ�ˮƽ��ʼλ��
    MOV     R2,  #0            ;// �м�����ʼֵ
    LDRH    R11, [R0, #PXx1]   ;// �������ڵ�ˮƽ��ʼλ��
    LDRH    R12, [R0, #PWx1]   ;// �������ڵ�ˮƽ���
    ADD     R12, R11, R12      ;// �������ڵ�ˮƽ����λ��
    ADD     R10,  R0,  #P_BUF  ;// �����ڻ�����ָ���ʼֵ

;//----------- ������ ----------//
Draw_Loop
    CMP     R2,  #0
    ITT     EQ
    BLEQ    Buld_0             ;// ���������л������ı������� 
    BEQ     Draw_Wave
    ADDS    R3,  R1, #2        ;// WIDTH+2
    CMP     R2,  R3
    ITT     EQ
    BLEQ    Buld_0             ;// ���������л������ı�������
    BEQ     Draw_Wave

    CMP     R2,  #1
    ITT     EQ
    BLEQ    Buld_1             ;// ���������л������ı�������
    BEQ     Draw_Wave
    ADDS    R3,  R1, #1        ;// WIDTH+1
    CMP     R2,  R3
    ITT     EQ
    BLEQ    Buld_1             ;// ���������л������ı�������
    BEQ     Draw_Wave

    SUBS    R3,  R2, #1
    MOVS    R6,  #25
    UDIV    R5,  R3, R6                
    MULS    R5,  R5, R6
    SUBS    R5,  R3, R5
    ITT     EQ
    BLEQ    Buld_4             ;// ���������л������ı�������
    BEQ     Draw_Wave

    MOVS    R6,  #5
    UDIV    R5,  R3, R6
    MULS    R5,  R5, R6
    SUBS    R5,  R3, R5
    ITT     EQ
    BLEQ    Buld_3             ;// ��������л������ı�������
    BEQ     Draw_Wave
    BL      Buld_2             ;// ���������л������ı�������

;//--------- ���������� --------//
Draw_Wave
    CMP     R2,  #3            ;// ��3~299
    BCC     Horizontal
    CMP     R2,  R1            ;// WIDTH
    BCS     Horizontal

    LDRH    R3,  [R0, #CH_A]   ;// ȡ CH_A �������ߵ�������־
    TST     R3,  #W_HID
    ITTT    EQ
    MOVEQ   R3,  #CCHA         ;// R3 = CH_A ����������ɫ��ƫ��
    ADDEQ   R4,  R0, #A_BUF
    BLEQ    Draw_Analog

    LDRH    R3,  [R0, #CH_B]   ;// ȡ CH_B �������ߵ�������־
    TST     R3,  #W_HID
    ITTT    EQ
    MOVEQ   R3,  #CCHB         ;// R3 = CH_B ����������ɫ��ƫ��
    ADDEQ   R4,  R0, #B_BUF
    BLEQ    Draw_Analog

    LDRH    R3,  [R0, #CH_C]   ;// ȡ CH_C �������ߵ�������־
    TST     R3,  #W_HID
    ITTT    EQ
    MOVEQ   R3,  #CCHC         ;// R3 = CH_C ����������ɫ��ƫ��
    ADDEQ   R4,  R0, #C_BUF
    BLEQ    Draw_Analog

;//------- ��ˮƽ�����α� ------//
Horizontal
    CMP     R2,  #0
    ITT     EQ
    BLEQ    Cursor_0           ;// �����л��α�˵�
    BEQ     Vertical
    ADDS    R3,  R1, #2        ;// WIDTH+2
    CMP     R2,  R3             
    ITT     EQ
    BLEQ    Cursor_0           ;// �����л��α�˵�
    BEQ     Vertical

    CMP     R2,  #1
    ITT     EQ
    BLEQ    Cursor_1           ;// �����л��α�˵�
    BEQ     Vertical
    ADDS    R3,  R1, #1        ;// WIDTH+1
    CMP     R2,  R3             
    ITT     EQ
    BLEQ    Cursor_1           ;// �����л��α�˵�
    BEQ     Vertical

    CMP     R2,  #2
    ITT     EQ
    BLEQ    Cursor_2           ;// �����л��α�˵�
    BEQ     Vertical
    CMP     R2,  R1            ;// WIDTH 
    IT      EQ
    BLEQ    Cursor_2           ;// �����л��α�˵�
    BEQ     Vertical
    BL      Cursor_3           ;// �����л��α���

;//------- ����ֱ�����α� ------//
Vertical
    BL      Cursor_4

;//--------- ���������� --------//

    LDRH    R3,  [R0, #POPF]   ;// ȡ�������ڵ�������־
    TST     R3,  #P_HID
    BNE     Send
    CMP     R2,  R11           ;// �жϵ��������д���ʼ
    BLT     Send
    CMP     R2,  R12           ;// �жϵ��������д������
    IT      LT
    BLLT    Draw_Pop           ;// �м����ڵ���������

;//--------- ��ʾ������ --------//
Send
    BL      Send_LCD           ;// �ӻ���������һ�����ݵ� LCD
    ADDS    R3,  R1, #2        ;// WIDTH+2
    CMP     R2,  R3             
    ITT     NE
    ADDNE   R2,  R2, #1
    BNE     Draw_Loop          ;// ������1����ʾ����

    POP     {R4-R12,PC}
;//=============================================================================
; Draw_Analog(R2:Col, R3:ColorNum, R4:pDat)   ��ģ�Ⲩ������  Used: R3-R7
;//=============================================================================
    NOP.W
    NOP.W
    NOP
Draw_Analog
    ADD     R4,  R4, R2
    LDRB    R5,  [R4]          ;// ȡ��ǰ�в����������� n1
    LDRB    R4,  [R4, #-1]     ;// ȡ��һ�в����������� n0
Analog0
    CMP     R4,  #199          ;// �϶˽�β R4 >= 200
    IT      HI
    BXHI    LR
    CMP     R4,  #0            ;// �¶˽�β R4 = 0
    IT      EQ
    BXEQ    LR

    CMP     R5,  R4
    ITTEE   CS                 ;// R5 = | n1 - n0 |
    MOVCS   R6,  R4
    SUBCS   R5,  R5, R4
    MOVCC   R6,  R5            ;// n1, n0 ����С���� R6
    SUBCC   R5,  R4, R5

    CMP     R6,  #198          ;// ��㳬�Ͻ������� R6 > 198
    IT      HI
    BXHI    LR
    ADDS    R4,  R5, R6
    CMP     R4,  #198          ;// �յ㳬�Ͻ����޷� R6 > 198
    IT      HI
    RSBHI   R5,  R6, #198
    BGT     Analog2

    CMP     R4,  #1            ;// �յ㳬�½������� R4 <= 1
    IT      LS
    BXLS    LR
    CMP     R6,  #2            ;// ��㳬�½����޷� R6 <= 2
    ITTT    LS
    MOVLS   R6,  #2
    SUBLS   R5,  R4, #2
    BLS     Analog2

    CMP     R5,  #0            ;// ˮƽ�߼Ӵ�
    ITT     EQ
    SUBEQ   R6,  R6, #1
    ADDEQ   R5,  R5, #2

Analog2
    CMP     R5,  #20           ;// ѡ����ɫ
    IT      GE
    ADDGE   R3,  R3, #18       ;// ѡ���������ɫ��
    LDRH    R3,  [R0, R3]

    LSL     R6,  R6, #1
    ADD     R6,  R0, R6        ;// ȷ����ʾλ��
Align00    
Analog3
    STRH    R3,  [R6], #2      ;// �����ε�
    SUBS    R5,  R5, #1
    BCS     Analog3
    BX      LR
;//=============================================================================
; Cursor_4(R1:pTab, R2:Col)// �����л��α�˵�  Used: R3-R8
;//=============================================================================
    NOP.W
Cursor_4
    MOVS    R3,  #P_TAB+6*2    ;// 6~8��Ϊ��ֱ�����α�
Cursor40
    MOV     R4,  R0
    LDRH    R5,  [R0, R3]      ;// ȡ���α������������־
    TST     R5,  #D_HID
    BNE     Cursor49           ;// ���α�����ת
Cursor41
    ADDS    R3,  R3, #9*2
    LDRH    R5,  [R0, R3]      ;// ȡ���α��������ʾλ��
    ADDS    R3,  R3, #9*2
    LDRH    R6,  [R0, R3]      ;// ȡ���α��������ʾ��ɫ
    SUBS    R3,  R3, #18*2     ;// �ָ�������ָ��

    SUBS    R8,  R5, #2
    CMP     R2,  R8            ;// �����������α�˵�
    BNE     Cursor42
    STRH    R6,  [R4]          ;// ����������
    ADDS    R4,  R4, #404
    STRH    R6,  [R4]          ;// ����������
    B       Cursor49
Cursor42
    ADDS    R8,  R8, #1
    CMP     R2,  R8            ;// ����������α�˵�
    BNE     Cursor43
    STRH    R6,  [R4], #2      ;// ����������
    STRH    R6,  [R4]          ;// �����±���
    ADDS    R4,  R4, #400
    STRH    R6,  [R4], #2      ;// �����ϱ���
    STRH    R6,  [R4]          ;// ����������
    B       Cursor49
Cursor43
    ADDS    R8,  R8, #1
    CMP     R2,  R8            ;// ���α�˵�, ���α���
    BNE     Cursor45
    STRH    R6,  [R4], #2      ;// ����������
    STRH    R6,  [R4], #2      ;// ���±��߸�
    STRH    R6,  [R4]          ;// ����������
    ADDS    R4,  R4, #396
    STRH    R6,  [R4], #2      ;// ����������
    STRH    R6,  [R4], #2      ;// ���ϱ���
    STRH    R6,  [R4]          ;// ����������

    LDRH    R5,  [R0, R3]      ;// ȡ���α������������־
    TST     R5,  #2
    BNE     Cursor45           ;// ���α���������ת
    MOVS    R4,  R0
    ADDS    R7,  R4, #400
Align01    
Cursor44
    STRH    R6,  [R4], #8      ;// ���α���
    CMP     R7,  R4
    BCS     Cursor44
    B       Cursor49
Cursor45
    ADDS    R8,  R8, #1
    CMP     R2,  R8            ;// ���ұ������α�˵�
    BNE     Cursor46
    STRH    R6,  [R4], #2      ;// ����������
    STRH    R6,  [R4]          ;// �����±���
    ADDS    R4,  R4, #400
    STRH    R6,  [R4], #2      ;// �����ϱ���
    STRH    R6,  [R4]          ;// ����������
    B       Cursor49
Cursor46
    ADDS    R8,  R8, #1
    CMP     R2,  R8            ;// �����������α�˵�
    BNE     Cursor49
    STRH    R6,  [R4]          ;// ����������
    ADDS    R4,  R4, #404
    STRH    R6,  [R4]          ;// ����������
Cursor49
    ADDS    R3,  R3, #1*2
    CMP     R3,  #P_TAB+9*2    ;//10
    BNE     Cursor40
    BX      LR
;//=============================================================================
; Cursor_3(R1:pTab, R2:Col)// ���������α���  Used: R3-R6
;//=============================================================================
    NOP
Align02    
Cursor_3
    MOVS    R3,  #P_TAB+5*2    ;// 0~5��Ϊˮƽ�����α�
    MOVS    R4,  R0
Cursor31
    LDRH    R5,  [R0, R3]      ;// ȡ���α������������־
    TST     R5,  #L_HID
    BNE     Cursor32           ;// ���α���������ת
    SUBS    R5,  R2, #1
    ANDS    R5,  R5, #3
    BNE     Cursor32           ;// ���α�������λ����ת
    ADDS    R3,  R3, #9*2
    LDRH    R5,  [R0, R3]      ;// ȡ���α��������ʾλ��
    ADDS    R4,  R0, R5
    ADDS    R3,  R3, #9*2
    LDRH    R6,  [R0, R3]      ;// ȡ���α��������ʾ��ɫ
    STRH    R6,  [R4]          ;// ���α���
    SUBS    R3,  R3, #18*2     ;// �ָ�������ָ��
Cursor32
    SUBS    R3,  R3, #1*2
    CMP     R3,  #P_TAB     
    BPL     Cursor31           ;// ������1���α�˵�
    BX      LR
;//=============================================================================
; Cursor_0(R1:pTab, R2:Col)// ���������α�˵�  Used: R3-R6
;//=============================================================================
    NOP
Align03
Cursor_0
    MOVS    R3,  #P_TAB+5*2    ;// 0~5��Ϊˮƽ�����α�
    MOVS    R4,  R0
Cursor01
    LDRH    R5,  [R0, R3]      ;// ȡ���α������������־
    TST     R5,  #1
    BNE     Cursor02           ;// ���α�˵�������ת
    ADDS    R3,  R3, #9*2
    LDRH    R5,  [R0, R3]      ;// ȡ���α��������ʾλ��
    ADDS    R4,  R0, R5
    ADDS    R3,  R3, #9*2
    LDRH    R6,  [R0, R3]      ;// ȡ���α��������ʾ��ɫ
    SUB     R4,  R4, #4
    STRH    R6,  [R4], #2
    STRH    R6,  [R4], #2
    STRH    R6,  [R4], #2      ;// ���������α�˵�
    STRH    R6,  [R4], #2
    STRH    R6,  [R4], #2
    SUBS    R3,  R3, #18*2     ;// �ָ�������ָ��
Cursor02
    SUBS    R3,  R3, #1*2
    CMP     R3,  #P_TAB       
    BPL     Cursor01           ;// ������1���α�˵�
    BX      LR
;//=============================================================================
; Cursor_1(R1:pTab, R2:Col)// ���������α�˵�  Used: R3-R6
;//=============================================================================
    NOP.W
Align04
Cursor_1
    MOVS    R3,  #P_TAB+5*2    ;// 0~5��Ϊˮƽ�����α�
    MOVS    R4,  R0
Cursor11
    LDRH    R5,  [R0, R3]      ;// ȡ���α������������־
    TST     R5,  #1
    BNE     Cursor12           ;// ���α�˵�������ת
    ADDS    R3,  R3, #9*2
    LDRH    R5,  [R0, R3]      ;// ȡ���α��������ʾλ��
    ADDS    R4,  R0, R5
    ADDS    R3,  R3, #9*2
    LDRH    R6,  [R0, R3]      ;// ȡ���α��������ʾ��ɫ
    SUBS    R4,  R4, #2
    STRH    R6,  [R4], #2
    STRH    R6,  [R4], #2      ;// ���������α�˵�
    STRH    R6,  [R4], #2
    SUBS    R3,  R3, #18*2     ;// �ָ�������ָ��
Cursor12
    SUBS    R3,  R3, #1*2
    CMP     R3,  #P_TAB     
    BPL     Cursor11           ;// ������1���α�˵�
    BX      LR
;//=============================================================================
; Cursor_2(R1:pTab, R2:Col)// ���������α�˵�  Used: R3-R6
;//=============================================================================
    NOP.W
    NOP.W
    NOP.W
    NOP
Align05
Cursor_2
    MOVS    R3,  #P_TAB+5*2    ;// 0~5��Ϊˮƽ�����α�
    MOVS    R4,  R0
Cursor21
    LDRH    R5,  [R0, R3]      ;// ȡ���α������������־
    TST     R5,  #1
    BNE     Cursor22           ;// ���α�˵�������ת
    ADDS    R3,  R3, #9*2
    LDRH    R5,  [R0, R3]      ;// ȡ���α��������ʾλ��
    ADDS    R4,  R0, R5
    ADDS    R3,  R3, #9*2
    LDRH    R6,  [R0, R3]      ;// ȡ���α��������ʾ��ɫ
    STRH    R6,  [R4]          ;// ���������α�˵�
    SUBS    R3,  R3, #18*2     ;// �ָ�������ָ��
Cursor22
    SUBS    R3,  R3, #1*2
    CMP     R3,  #P_TAB   
    BPL     Cursor21           ;// ������1���α�˵�
    BX      LR
;//=============================================================================
;// R0:pDat, R1:pTab, R2:Col, R3:Var, R4:pBuf, R5:Cnt, R6:Tmp,
;//=============================================================================
; void Fill_Base(R3 = u32 Color)// �л��������ɫ RET: R4=R0+2   Used: R3-R5
;//=============================================================================
    NOP
    EXPORT  Fill_Base
Fill_Base
    MOV.W   R4,  R0
    MOV.W   R5,  #102          ;// 1+202��/2 ��404 Bytes
Align06    
Fill_Loop_0
    STR     R3,  [R4], #4      ;// ������ɺ�ָ���4
    SUBS    R5,  #1
    BNE     Fill_Loop_0
    ADD     R4,  R0, #2        ;// ָ�����
    MOV     R3,  #GRID_COLOR   ;// Ԥװ��������ɫֵ
    BX      LR
;//=============================================================================
; Draw_Pop(R0:pWork) // ����������                                  Used: R3-R8
;//=============================================================================
    NOP.W
    NOP.W
    NOP.W
    NOP
Align07
Draw_Pop
    LDRH    R5,  [R0, #PYx2]   ;// ȡ�������ڵĴ�ֱ��ʼλ��
    ADDS    R5,   R0,  R5
    LDRH    R6,  [R0, #PHx2]   ;// ȡ�������ڵĴ�ֱ�߶�
    ADD     R7,   R0, #PHx2    ;// HYx2+2 �Ǵ����ɫ��ָ�� CPTR
Pop_Loop
    LDRH    R4,  [R10], #2     ;// ȡ Pop ����(˫�ֽڹ�4����)
    ANDS    R3,  R4,  #0x0E
    ITT     NE                 ;// ��0��͸��ɫ������
    LDRHNE  R3,  [R7, R3]      ;// ���ȡ1~7��ɫ
    STRHNE  R3,  [R5]          ;// ����1��
    ADDS    R5,  R5,  #2
    LSR     R4,  R4,  #4
    ANDS    R3,  R4,  #0x0E
    ITT     NE                 ;// ��0��͸��ɫ������
    LDRHNE  R3,  [R7, R3]      ;// ���ȡ1~7��ɫ
    STRHNE  R3,  [R5]          ;// ����2��
    ADDS    R5,  R5,  #2
    LSR     R4,  R4,  #4
    ANDS    R3,  R4,  #0x0E
    ITT     NE                 ;// ��0��͸��ɫ������
    LDRHNE  R3,  [R7, R3]      ;// ���ȡ1~7��ɫ
    STRHNE  R3,  [R5]          ;// ����3��
    ADDS    R5,  R5,  #2
    LSR     R4,  R4,  #4
    ANDS    R3,  R4,  #0x0E
    ITT     NE                 ;// ��0��͸��ɫ������
    LDRHNE  R3,  [R7, R3]      ;// ���ȡ1~7��ɫ
    STRHNE  R3,  [R5]          ;// ����4��
    ADDS    R5,  R5, #2
    SUBS    R6,  R6, #8
    BNE     Pop_Loop
    BX      LR                 ;// ������ʾ���
    NOP
;//=============================================================================
; void Buld_0(R4 = u16* pCol)   // ���������л������ı������� Used: R3-R5
;//=============================================================================
Buld_0
    MOV     R3,  #BACKGROUND   ;// ������ɫ
    B       Fill_Base
;//=============================================================================
; void Buld_2(R4 = u16* pCol)   // ���������л������ı������� Used: R3-R6
;//=============================================================================
Buld_2
    MOV     R6,  LR
    MOV     R3,  #BACKGROUND   ;// ������ɫ
    BL      Fill_Base
    STRH    R3,  [R4, #400]    ;// �ϱ���
    STRH    R3,  [R4]          ;// �±���
    BX      R6
;//=============================================================================
; void Buld_4(R4 = u16* pCol)   // ���������л������ı�������
;//=============================================================================
    NOP.W
    NOP
Buld_4
    MOV     R6,  LR
    MOV     R3,  #BACKGROUND   ;// ������ɫ
    BL      Fill_Base
    MOVS    R5,  #41           ;// 41��  P_TAB
Align08    
Loop7
    STRH    R3, [R4], #5*2     ;// ÿ5�л�1���
    SUBS    R5,  R5,  #1
    BNE     Loop7
    BX      R6
;//=============================================================================
; void Buld_3(R4 = u16* pCol)   // ��������л������ı������� Used: R3-R6
;//=============================================================================
    NOP.W
    NOP.W
    NOP
Buld_3
    MOV     R6,  LR
    MOV     R3,  #BACKGROUND   ;// ������ɫ
    BL      Fill_Base
    MOVS    R5,  #9            ;// 9���
Align09    
Loop3
    STRH    R3, [R4], #50      ;// ÿ25�л�1���
    SUBS    R5,  R5,  #1
    BNE     Loop3
    BX      R6
;//=============================================================================
; void Buld_1(R4 = u16* pCol)   // ���������л������ı������� Used: R3-R6
;//=============================================================================
Buld_1
    MOV     R6,  LR
    MOV.W   R3,  #GRID_COLOR
    MOVT    R3,  #GRID_COLOR   ;// Ϊ��ߴ���Ч�ʣ�ȡ32bits������ɫ
    BL      Fill_Base          ;// RET: R4=R0+2
    MOV     R3,  #BACKGROUND   ;// ������ɫ
    STRH    R3,  [R4, #402]            
    STRH    R3,  [R4, #-2]     ;// �±��������հ�
    BX      R6
;//=============================================================================
; void __Mem32Fill(u32* pMem, u32 Data, u32 n)
;//=============================================================================
    NOP.W
    NOP.W
    NOP
    EXPORT  __Mem32Fill
Align10    
__Mem32Fill
    STR     R1, [R0], #4
    SUBS    R2, R2, #1  
    BNE     __Mem32Fill  
    BX      LR         
;//=============================================================================
; void Send_LCD(u16* pBuf, u16 Row) // ��һ�л��������ݵ� LCD     Used: R3-R8
;//=============================================================================
    NOP.W
    NOP
    EXPORT  Send_LCD
Send_LCD
    MOVS    R5,  R0
    PUSH    {R0-R3, LR}
    LDRH    R1, [R0, #M_Y0]
    LDRH    R0, [R0, #M_X0]
    ADDS    R0,  R0, R2
    BL      __SetPosi
    MOVS    R0, #0x0C00
    MOVT    R0, #0x4800        ;// Port D Base 0x48000C00
    MOVS    R1, #WR 
    MOVS    R2, #203           ;// 1+202
Align11    
Loop9    
    LDRH    R3, [R5], #2
    STRH    R3, [R0,  #0x414]  ;// Port E ODR
    STRH    R1, [R0,  #0x028]  ;// Port D BRR
    SUBS    R2,  R2,  #1  
    STRH    R1, [R0,  #0x018]  ;// Port D BSRR
    BNE     Loop9
    POP     {R0-R3, PC}
;//=============================================================================
   END

;******************************* END OF FILE ***********************************


