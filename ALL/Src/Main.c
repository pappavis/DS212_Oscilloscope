/********************* (C) COPYRIGHT 2017 e-Design Co.,Ltd. ********************
File Name : main.c
Version   : DS212                                                 Author : bure
*******************************************************************************/
#include "Version.h"
#include "Process.h"
#include "Drive.h"
#include "Func.h"
#include "Draw.h"
#include "Bios.h"
#include "Menu.h"
#include "Disk.h"
#include "LCD.h"
#include "FAT12.h"
#include "File.h"
#include "Math.h"

/******************************************************************************* 

 
*******************************************************************************/

typedef void (*pFunc)(void);
void MSD_Disk_Config(void);
void Set_Licence(u16 x, u16 y);

//===============================APP�汾��======================================
u8  APP_VERSION[] = "V1.03";   //���ó���12���ַ�

u16 Key_Flag = 0; 
u8  CalPop_Flag = 1, ResPop_Flag = 1; 
u8  Menu_Temp[5], NumStr[20];
u16 FileInfo,     Label_Cnt;
u16 temp = 0;
u8  Channel = 0;

void main(void)
{
  //===============================ϵͳ��ʼ��===================================
  __Ctrl(SYS_CFG, RCC_DEV | TIM_DEV | GPIO_OPA | ADC_DAC | SPI );

 GPIO_SWD_NormalMode() ;  //�ر�SWD��¼�ڹ���
 
#if   defined (APP1)  
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000);      //��һ����APP
#elif defined (APP2)  
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);     //�ڶ�����APP
#endif  
  SysTick_Config(SystemCoreClock/1000);                 //SysTick = 1mS
  __Ctrl(B_LIGHT, 50);                                  //�� LCD �������� 50%

  __Ctrl(BUZZVOL, 50);                                  //�趨����������(0~100%) 
  Beep(200);                                            //200mS                                     
  
  USB_MSD_Config();
  Init_Fat_Value();
  __Ctrl(SMPL_ST, DISABLE);
  __Ctrl(SMPL_ST, SIMULTANEO);
  __Ctrl(SMPLBUF, (u32)Smpl);
  __Ctrl(SMPLNUM, DEPTH[PopMenu1_Value[WIN_Depth]]);
  __Ctrl(SMPL_ST, ENABLE);
  
  //=============================��ʾ������ʾ��Ϣҳ��===========================  
  SetColor(BLK, WHT);
  DispStr(0,      90, PRN, "                                        ");
  DispStr(0,      70, PRN, "                                        ");
#if   defined (APP1)
  DispStr(8,      90, PRN, "       Oscilloscope  APP1");
#elif defined (APP2) 
  DispStr(8,      90, PRN, "       Oscilloscope  APP2"); 
#endif  
  DispStr(8+26*8, 90, PRN,                            APP_VERSION);
  DispStr(8,      70, PRN, "        System Initializing...       ");
  __Ctrl(DELAYmS, 500); 
  
  ADC_StartConversion(ADC1);  
  ADC_StartConversion(ADC3);
  ADC_StartConversion(ADC2);  
  ADC_StartConversion(ADC4);
  memset(VRAM_PTR+TR1_pBUF, ~0,    900); 
  memcpy(VRAM_PTR+TAB_PTR,  PARAM,  90);  
  
  //=============================��һ��д�̼��Զ�У׼===========================   
  Read_CalFlag();
  /**/
  if(Cal_Flag == 1){  
    Cal_Flag = 0;
    SetColor(BLK, WHT);
    DispStr(8, 90, PRN, "                                        ");
    DispStr(8, 70, PRN, "                                        ");
    DispStr(8, 90, PRN, "      Run the calibration program...    ");
    DispStr(8, 70, PRN, "        Please wait a few seconds       ");
    Zero_Align();                              //��ֱλ�����У�� 
    Restore_OrigVal();                         //���ò���
    Save_Param();                              //�������
    Save_Kpg();
  }
  
  //=============================��������ʾ=====================================   
  Read_Kpg();                                   //��ȡУ׼����
  Load_Param();                                 //��U�̶�ȡ�������
  File_Num();                                   //��ȡ�ļ����
  ClrScrn(DAR);                                 //��������
  menu.menu_flag = 1;                           //menu���˵�
  menu.mflag  |= UPD;                           //menuѡ��
  menu.iflag  |= UPD;                           //menu�Ӳ˵�
  Label_Flag  |= UPD;                           //�α�
  Show_Title();                                 //��ʾ
  Show_Menu();                                  //���˵���ѡ��   
  Update_Proc_All();                            //����ˢ��
  Update_Label();                               //�α���ʾ
  Print_dT_Info(INV);
  Print_dV_Info(INV);
  Battery_Show();              
  MenuCnt = 5000;                               //Menu���˵���һ�δ���ʱ��
  PD_Cnt      = PopMenu3_Value[SYS_Standy]*Unit;   //��Ļ����ʱ��
  AutoPwr_Cnt = PopMenu3_Value[SYS_PowerOff]*Unit; //�Զ��ػ�ʱ��
  if(PopMenu1_Value[TRI_Fit])Key_S_Time = 300;
  else                       Key_S_Time = 0;
  
  //========================��� Licence ��ȷ��ر�DEMO��=======================   
  if(__Info(LIC_OK) == 1){  
    PopType &= ~DEMO_POP;  
    ClosePop();
  }
  else Demo_Pop();

  Keys_Detect();                                //����
  KeyIn=0;
    
  //===================================��ѭ������===============================
  while(1){
    //=================��ADC����=====================   

    if(!__Info(LIC_OK)){//�ػ����� KEY_R KEY_S
      if(((~GPIOB->IDR)& 0x0020)&&((~GPIOB->IDR)& 0x0040))
        __Ctrl(PWROFF, ENABLE);
    }
    //====================����=======================
    if(((PopMenu3_Value[SYS_Standy]!=0) && (PD_Cnt == 0))){
      __Ctrl(B_LIGHT,1);                   //�رձ��� 
      StdBy_Flag = 1;
    }
    //==================�Զ��ػ�=====================
    if((PopMenu3_Value[SYS_PowerOff] != 0) && (AutoPwr_Cnt == 0) && (__Info(P_VUSB) == 0)){
      Beep(500);
      __Ctrl(DELAYmS, 500);                //�ػ�ǰ��������0.5s��ʾ 
      __Ctrl(PWROFF,  ENABLE);              //power off
    }
    else if(__Info(P_VUSB))                 //USB���ʱ���Զ��ػ�
      AutoPwr_Cnt = PopMenu3_Value[SYS_PowerOff]*Unit;
    
    //if(menu.menu_flag == 1)Show_Menu();
    if((PopType & DEMO_POP)&&!(PopType&(DAILOG_POP|PWR_POP|LIST_POP|FILE_POP)))
      MovePop();                           //δ��������ʾDemo����    
    if(About_Flag == 0){                   //��ʾAboutʱ����ˢ�²��δ���
      Process();
      __DrawWindow(VRAM_PTR);
    }
    
    Keys_Detect();                         //����ɨ��
   
    if(KeyIn) Key_Flag = 1;
    else      Key_Flag = 0;
    
    if(About_Flag == 1){                   //��ʾAboutʱ��ֻ�С�M�����ͽ�ͼ��Ч
      Key_Flag = 0;
      if((KeyIn == K_M)||(KeyIn == R_HOLD))
        Key_Flag = 1;
      
    }
    
    if(StdBy_Flag == 1){
      if(KeyIn)
        Key_Flag = 1;
      else
        Key_Flag = 0;
    }

    if(KeyIn && Key_Flag){       
      
      //==========�а����ָ��������Զ��ػ�ʱ��===============
      if(((PopMenu3_Value[SYS_Standy] != 0) &&(PD_Cnt == 0)) || (StdBy_Key == 1)){
        __Ctrl(B_LIGHT, PopMenu3_Value[SYS_BKLight]*10);
        StdBy_Flag  = 0;
        StdBy_Key   = 0;
        KeyIn = 0;
      }
      PD_Cnt      = PopMenu3_Value[SYS_Standy]*Unit;   //����ʱ�� 
      AutoPwr_Cnt = PopMenu3_Value[SYS_PowerOff]*Unit; //�Զ��ػ�ʱ��
      
      //=======================��������=====================
      switch (KeyIn){
        //-------------��е����---------------- 

      case R_HOLD:                       //����RUN������ͼ��ݰ���
        Beep(50); 
        __Ctrl(DELAYmS, 100);
        Beep(50); 
        FileInfo = Save_Bmp(PopMenu3_Value[SAVE_Bmp]);
        DispFileInfo(FileInfo);
        
        SetColor(DAR, ORN);
        Print_dT_Info(INV);                //��ʾT1-T2
        Update[T1F] &=~ UPD ;
        break;
        
      case M_HOLD:                        //����
        if(menu.menu_flag == 1)
        {
          if(PopType & (FILE_POP)){ 
            ClosePop(); 
            menu.current = Menu_Temp[0];
            menu.menu_index[menu.current] = Menu_Temp[1];
            break;          
          }
          else if(!(PopType & ( DAILOG_POP))){ 
            Menu_Temp[0] = menu.current;
            Menu_Temp[1] = menu.menu_index[menu.current];
            
            ClosePop();   
            PopCnt = POP_TIME;                  //�趨 Pop �Զ��رն�ʱ 5000mS
            menu.current = Option;
            menu.menu_index[menu.current] = 0;
            Cur_PopItem = 1;
            Show_PopMenu(Cur_PopItem);      //�����Ӳ˵�_Pop
            List_Pop();
          }
        }
        break;
        
      case S_HOLD: 
        if(!(PopType & (LIST_POP|DAILOG_POP|FILE_POP))){
          
          if(menu.menu_flag){                          //���ز˵�����
            MenuCnt = 0;
            menu.menu_flag = 0;
            ParamTab[M_WX] = 300;
            Clear_Label_R(DAR);                        //��������
          }
          else {                                        //�����˵�����
            if(__Info(LIC_OK) == 1);                   //�л�����ʱdemo��λ
            else if((ParamTab[PXx1]+ParamTab[PWx1]) >= (WIDTH_MINI+1))Demo_Pop();
            ParamTab[M_WX] = WIDTH_MINI;
            menu.menu_flag = 1;
            Show_Menu();
            menu.mflag |= UPD;
          }
        }
        break;
    
      case K_RUN:                     //RUN��
        {                             //����ͣ������
          if(Status == STOP) {
            Status &= ~STOP;
            if(PopMenu1_Value[TRI_Sync] == SNGL)ADC_Start();
            if(PopMenu1_Value[TRI_Sync] == NORM)ADC_Start();
            Norm_Clr = 1;
            SNGL_Kflag = 1;
            Update_Proc_All();
          }
          else  {
            Status  |=  STOP;
            Ch1_Posi = PopMenu1_Value[CH1_Posi];
            Ch2_Posi = PopMenu1_Value[CH2_Posi];
            Ch3_Posi = PopMenu1_Value[CH3_Posi];
          }
          Update_Status();
        }

        break; 
        
      case K_S:
        if((PopType & PWR_POP)){          //�ػ�����
        }
        else if((PopType & FILE_POP)&&(menu.current == Option)  //�ļ��Ӳ˵�����
                &&(menu.menu_index[menu.current] == 0)){
                  FileInfo = 1;
                  if(Cur_PopItem == SAVE_PAM) {
                    menu.current = Menu_Temp[0];
                    menu.menu_index[menu.current] = Menu_Temp[1];
                    Save_Param();
                    FileInfo = 0;
                    DispFileInfo(FileInfo);
                    menu.current = Option;
                    menu.menu_index[menu.current] = 0;
                    Show_PopMenu(Cur_PopItem);
                    break;
                  }
                  else if(Cur_PopItem == SAVE_BMP) {
                    ClosePop();
                    __DrawWindow(VRAM_PTR);
                    FileInfo = Save_Bmp(PopMenu3_Value[SAVE_Bmp]); 
                    List_Pop();
                  }
                  else if(Cur_PopItem == SAVE_DAT) {
                    FileInfo = Save_Dat(PopMenu3_Value[SAVE_Dat]);
                  }
                  else if(Cur_PopItem == SAVE_BUF) {
                    FileInfo = Save_Buf(PopMenu3_Value[SAVE_Buf]);
                  }
                  else if(Cur_PopItem == SAVE_CSV) {
                    FileInfo = Save_Csv(PopMenu3_Value[SAVE_Csv]);
                  }
                  else if(Cur_PopItem == LOAD_DAT) {
                    FileInfo = Load_Dat(PopMenu3_Value[LOAD_Dat]);
                  }
                  else if(Cur_PopItem == LOAD_BUT) {
                    FileInfo = Load_Buf(PopMenu3_Value[LOAD_Buf]);
                  }
                  else if(Cur_PopItem == SAVE_SVG) {
                    FileInfo = Save_Svg(PopMenu3_Value[SAVE_Svg]);
                    menu.current = Option;              //����File����
                    menu.menu_index[menu.current] = 0;
                    Show_PopMenu(Cur_PopItem);
                    List_Pop();
                  }
                  
                  Show_PopMenu(Cur_PopItem);
                  Show_Title();
                  DispFileInfo(FileInfo);
                  if(PopType & DAILOG_POP)  ClosePop();
                  break;
                }
        
        else if((menu.current == Option) && (menu.menu_index[menu.current] == 3)
                && (PopType & (LIST_POP |DAILOG_POP))){   //CALУ׼ѡ��
                  if(Cur_PopItem == CAL_ZERO) {
                    if(CalPop_Flag == 1){
                      Dialog_Pop("Auto Calibration?");
                      PopCnt = POP_TIME;
                      CalPop_Flag = 0;
                      break;
                    }
                    if(CalPop_Flag == 0){
                      if(PopType & DAILOG_POP){          //DAILOG_POP�Ի���ѡ��
                        Save_Kpg(); 
                        ClosePop();
                        CalPop_Flag = 1;
                      }
                      else if(PopType & LIST_POP){       //MENU_POP�Ի���ѡ��
                        if(Cur_PopItem == CAL_ZERO) {
                          ClosePop();
                          Tips_Pop("Waiting for Calibration ...");
                          __DrawWindow(VRAM_PTR);        //ˢ�½���
                          Zero_Align();
                          Update_Proc_All();
                          ClosePop();
                          Dialog_CalPop("Cal completed,Save data?",48,110,32,26*6);
                          PopCnt = POP_TIME;
                        }
                      }
                    }
                  }
                  else if(Cur_PopItem == RES_DATA) {
                    if(ResPop_Flag ==1){
                      Dialog_Pop("  Restore Data ?");
                      PopCnt = POP_TIME;
                      ResPop_Flag = 0;
                      break;
                    }
                    if(ResPop_Flag == 0){
                      if(PopType & DAILOG_POP){
                        menu.current = Oscillo;
                        menu.menu_index[Oscillo] = 0;
                        Save_Param();
                        ClosePop();
                        menu.mflag |= UPD;               //menuѡ��
                        Show_Menu();                     //���˵���ѡ�� 
                        ResPop_Flag = 1;
                      }
                      else if(PopType & LIST_POP){
                        Restore_OrigVal();
                        menu.current = Option;
                        menu.menu_index[Option] = 3;
                        Show_Title();                   //��ʾ
                        Show_Menu();                    //���˵���ѡ��   
                        Update_Proc_All();              //����ˢ��
                        ClosePop();
                        Dialog_CalPop(" Restored,Save Setting?", 90, 50, 32, 26*6);
                        PopCnt = POP_TIME;                
                      }
                    }
                  }
                  break;
                }
        else if(!(PopType & LIST_POP)){ 
          if((menu.menu_flag == 1)){
            ParamTab[M_WX] = 251;
            //if(menu.menu_flag == 1){
              if(menu.current >= MENU_MAX)menu.current = 0;
              else menu.current++;
            //}
            //menu.menu_flag = 1;
            Show_Menu();
            menu.mflag |= UPD;  
            MenuCnt = 6000;  
          }
          else{
            PopMenu1_Value[TRI_Ch] = !PopMenu1_Value[TRI_Ch] ;
            Update[VTF] |= UPD;
            Label_Flag |= UPD;
          }
        }
        
        
        break;
      
      case KEY_DOUBLE_M:
        Beep(50); 
        __Ctrl(DELAYmS, 100);
        Beep(50); 
        if(PopMenu1_Value[TRI_Fit]){
          Auto_Fit();
          menu.iflag |= UPD;
        }
        break;
        
      case K_M:
        
        if(menu.menu_flag){
          if(PopType & PWR_POP){           //�ڹػ������£�����Դ���رմ���
            PopType &= ~PWR_POP;
            ClosePop();
          }
          
          else if(!(PopType & (LIST_POP|DAILOG_POP|FILE_POP))){ 
            //���Ӵ���ʱ�����Ӵ���
            PopCnt = POP_TIME;                 // �趨 Pop �Զ��رն�ʱ 5000mS
            Cur_PopItem = 1;               // �Ӵ���Ĭ��ѡ��Ϊ��һѡ��
            Show_PopMenu(Cur_PopItem);     //�����Ӳ˵�_Pop
            if(PopType & FILE_POP){        //��������ļ������Ӵ��ڣ���¼��ǰҳ�͵�ǰѡ��
              Menu_Temp[0] = menu.current;
              Menu_Temp[1] = menu.menu_index[menu.current];
            }
            if((menu.menu_index[menu.current] != 5)||(menu.current == 0))
              List_Pop();                  //��ص�ѹ��about����������
          }
          
          else if(PopType & (LIST_POP|DAILOG_POP|FILE_POP)){ 
            //���Ӵ���ʱ���ر��Ӵ���          
            if(PopType & FILE_POP){        //�ļ������Ӵ���,�ָ���ʱ�ĵ�ǰҳ��ѡ��
              menu.current = Menu_Temp[0];
              menu.menu_index[menu.current] = Menu_Temp[1];
            } 
            ClosePop();
            CalPop_Flag  = 1;             //Auto_Cal?
            ResPop_Flag  = 1;             //Restore?
            Windows_Flag = 0;             //�ر�windows
            Update_Windows();       
          }
        }
        else{
          if(Channel == CH1_Vol){
            Channel = CH2_Vol;
            CHA_Col = DAR;
            CHB_Col = RED_;
          }
          else{
            Channel = CH1_Vol;
            CHA_Col = RED_;
            CHB_Col = DAR;
          }
        }
        break;
        
      case K_UP:
        if(menu.menu_flag == 0){
          if(PopMenu1_Value[Channel]<Popmenu1_Limit1[Channel])
            PopMenu1_Value[Channel]++;
          break;
        }
        
        if((PopType & LIST_POP)|| (Windows_Pop == 1)){   //�Ӳ˵�Popѡ��
          if((menu.current == Option) && (menu.menu_index[menu.current] == 1) 
             && (PopMenu3_Value[WAVE_Type] > 0)){
               if(Cur_PopItem <= 1)           //menu_key_chose
               {
                 if(PopMenu3_Value[SYS_PosiCyc])Cur_PopItem = Cur_Limit-1 ;  //ģ�����ʱ��DUTY��ѭ��
               }else Cur_PopItem--;            
             }else{
               if(Cur_PopItem <= 1){           //menu_key_chose
                 if(PopMenu3_Value[SYS_PosiCyc])Cur_PopItem = Cur_Limit ;
               }
               else Cur_PopItem--;
             }
          Show_PopMenu(Cur_PopItem);
          menu.mflag &= ~UPD;
          menu.iflag &= ~UPD;
      CalPop_Flag = 1;
          ResPop_Flag = 1;
        }
        else if(PopType & FILE_POP){         //�ļ�����Popʱ����ѡ��
          menu.current = Option;
          menu.menu_index[menu.current] = 0;
          if(Cur_PopItem <= 1){               //menu_key_chose
            if(PopMenu3_Value[SYS_PosiCyc])Cur_PopItem = Cur_Limit ;
          }
          else Cur_PopItem--;
          Show_PopMenu(Cur_PopItem);
          menu.mflag &= ~UPD;
          menu.iflag &= ~UPD;
        }
        else if(!(PopType & (DAILOG_POP | PWR_POP))){     //���˵�ѡ��
          if(menu.menu_index[menu.current] <= 0){          //menu_key_chose
            if(PopMenu3_Value[SYS_PosiCyc])menu.menu_index[menu.current] = Menu_Limit[menu.current]-1;
          }
          else 
            menu.menu_index[menu.current]--;
          menu.iflag |= UPD;
        }
        break;
        
      case K_DOWN:
        if(menu.menu_flag == 0){
          if(PopMenu1_Value[Channel]>PopMenu1_Limit2[Channel])
          PopMenu1_Value[Channel]--;
          break;
        }
        
        if((PopType & LIST_POP)|| (Windows_Pop == 1)){
          if((menu.current == Option)&&
             (menu.menu_index[menu.current] == 1)
               &&(PopMenu3_Value[WAVE_Type] > 0)){
                 if(Cur_PopItem >= Cur_Limit-1){           //menu_key_chose
                   if(PopMenu3_Value[SYS_PosiCyc])Cur_PopItem = 1 ;
                 }
                 else Cur_PopItem++;
                 
               }else{
                 if(Cur_PopItem >= Cur_Limit  ){           //menu_key_chose
                   if(PopMenu3_Value[SYS_PosiCyc])Cur_PopItem = 1;
                 }
                 else 
                   Cur_PopItem++;
               }
          Show_PopMenu(Cur_PopItem);
          menu.mflag &= ~UPD;
          menu.iflag &= ~UPD;
          CalPop_Flag = 1;
          ResPop_Flag = 1;
        }
        else if(PopType & FILE_POP){                      //�ļ�����Popʱ����
          menu.current = Option;
          menu.menu_index[menu.current] = 0;
          if(Cur_PopItem >= Cur_Limit  ){                  //menu_key_chose
            if(PopMenu3_Value[SYS_PosiCyc])Cur_PopItem = 1;
          }
          else 
            Cur_PopItem++;
          Show_PopMenu(Cur_PopItem);
          menu.mflag &= ~UPD;
          menu.iflag &= ~UPD;
        }
        else if(!(PopType & (DAILOG_POP | PWR_POP))){
          if(menu.menu_index[menu.current] >= Menu_Limit[menu.current]-1){
            if(PopMenu3_Value[SYS_PosiCyc])menu.menu_index[menu.current] = 0;            //menu_key_chose
          }
          else 
            menu.menu_index[menu.current]++;
          menu.iflag |= UPD;
        }
        break;        
        
      case K_LEFT:
        if(menu.menu_flag == 0){
          temp = PopMenu1_Value[TIM_Base];
          if(PopMenu1_Value[TIM_Base]>PopMenu1_Limit2[TIM_Base])
            PopMenu1_Value[TIM_Base]--;
          if((temp==2)&&(PopMenu1_Value[TIM_Base]==1))__Ctrl(SMPL_MODE, INTERLEAVE);
          break;
        }
        
        if(PopType & FILE_POP) {                          //�ļ�����Popʱ����
          PMenu_Proc(dec, Cur_PopItem, 0);
          menu.mflag &= ~UPD;
          menu.iflag &= ~UPD;
        }
        else {
          if((PopType & LIST_POP)|| (Windows_Pop == 1)) {
            
            PMenu_Proc(dec, Cur_PopItem, 0);
            CalPop_Flag = 1;
            ResPop_Flag = 1; 
            if(menu.current == Oscillo){
              while(__Info(KEY_IN)==0x80){
                PMenu_Proc(dec, Cur_PopItem, 0);
                CalPop_Flag = 1;
                ResPop_Flag = 1;
                Process();
                __DrawWindow(VRAM_PTR);
                Update_Label();
              }
            }
          }
          else if(!(PopType & (DAILOG_POP | PWR_POP)))  Item_Proc(dec);
        }
        break;
        
      case K_RIGHT:
        if(menu.menu_flag == 0){
          temp = PopMenu1_Value[TIM_Base];
          if(PopMenu1_Value[TIM_Base]<Popmenu1_Limit1[TIM_Base])
            PopMenu1_Value[TIM_Base]++;
          if((temp==1)&&(PopMenu1_Value[TIM_Base]==2))__Ctrl(SMPL_MODE, SIMULTANEO);
          break;
        }
        
        if(PopType & FILE_POP) {                      //�ļ�����Popʱ����
          PMenu_Proc(add, Cur_PopItem, 0);
          menu.mflag &= ~UPD;
          menu.iflag &= ~UPD;
        }
        else{
          if((PopType & LIST_POP) || (Windows_Pop == 1)) {
            PMenu_Proc(add, Cur_PopItem, 0);
            CalPop_Flag = 1;
            ResPop_Flag = 1;
            if(menu.current == Oscillo){
              while(__Info(KEY_IN)==0x80){
                PMenu_Proc(add, Cur_PopItem, 0);
                CalPop_Flag = 1;
                ResPop_Flag = 1;
                Process();
                __DrawWindow(VRAM_PTR);
                Update_Label();
              }
            }
          }
          else if(!(PopType & (DAILOG_POP | PWR_POP)))   Item_Proc(add);
        }
        break;
        
      }//----switch end-----
      Beep(50);
      
      KeyIn = 0;
      
      if(menu.menu_flag == 1)Show_Menu();    //�в˵���ʱˢ��
      else {                                 //�޲˵���ʱˢ��
        Update_Proc_All();
        Show_Title();
      }
      Update_Label();
    }//---Key_In end------
    
    
    if(Bat_Vol() < 3200)//���С��3.2V�Զ��ػ�
    {
      Battery = 0;
      Battery_update();
      Beep(500);
      __Ctrl(DELAYmS, 500);                //�ػ�ǰ��������0.5s��ʾ 
      __Ctrl(PWROFF, ENABLE);
    }
    
    if(About_Flag == 0){                             //�������ݶ�ʱˢ��
      if((Label_Cnt == 50)){
        Label_Cnt = 0;
        Label_Flag |= UPD;
        Update_Label();   
        Print_dT_Info(INV);
        Print_dV_Info(INV);
        Battery_update();
        if((menu.menu_flag == 1) && (menu.current == Measure))Show_Measure();      
      }else Label_Cnt++;
    }
  }
}

/******************************** END OF FILE *********************************/
