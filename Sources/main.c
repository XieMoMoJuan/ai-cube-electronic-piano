//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// 在此添加用户文件头说明信息
// 文件名称: main.c
// 文件描述:
// 文件版本: V1.0
// 修改记录:
//   1. (2025-12-03) 创建文件
////////////////////////////////////////
//<<AICUBE_USER_HEADER_REMARK_END>>

#include "config.h" //默认已包含stdio.h、intrins.h等头文件

//<<AICUBE_USER_INCLUDE_BEGIN>>
// 在此添加用户头文件包含
//<<AICUBE_USER_INCLUDE_END>>

//<<AICUBE_USER_GLOBAL_DEFINE_BEGIN>>
#define ADC_COMPARE_VALUE 0x999

uint16_t adcValue = 0;
volatile uint8_t isOpenPWM = 0;
volatile uint16_t pwm_counter = 0;
volatile uint16_t pwm_half_period_count = 0;

sbit PWM_PIN = P1 ^ 0;
volatile uint8_t triggered = 0;
uint8_t ch = 0;
uint8_t noteIndex = 0;

// C5 ~ B5
const uint16_t noteFreq[7] = {
    523, // C5
    587, // D5
    659, // E5
    698, // F5
    784, // G5
    880, // A5
    988  // B5
};

// 定义 ADC通道 -> 音符索引 映射表
// const uint8_t adcChannelToNote[15] = {
//    0xFF, 0xFF, 0xFF, 0xFF,  // 0~3 未用
//    0xFF, 0xFF, 0xFF, 0xFF,  // 4~7 未用
//    5,    // 8 -> F5
//    6,    // 9 -> G5
//    7,    // 10 -> A5
//    8,    // 11 -> B5
//    0,    // 12 -> C5
//    1,    // 13 -> D5
//    2     // 14 -> E5
//};
const uint8_t adcChannelToNote[15] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0~7 未用
    3,                                              // 8 -> so
    4,                                              // 9 -> la
    5,                                              // 10 -> xi
    6,                                              // 11 -> do
    0,                                              // 12 -> re
    1,                                              // 13 -> mi
    2                                               // 14 -> fa
};

// 设置 PWM 频率：freq 单位为 Hz
void PWM_SetFreq(uint16_t freq)
{
  // Timer0 中断周期 = 20us
  // half period = 1/(2*freq)
  // 需要的中断次数 = half_period / 20us
  pwm_half_period_count = 25000 / freq;

  if (pwm_half_period_count < 1)
    pwm_half_period_count = 1;

  pwm_counter = 0;
}

//<<AICUBE_USER_GLOBAL_DEFINE_END>>

////////////////////////////////////////
// 项目主函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void main(void)
{
  //<<AICUBE_USER_MAIN_INITIAL_BEGIN>>
  // 在此添加用户主函数初始化代码
  //<<AICUBE_USER_MAIN_INITIAL_END>>

  SYS_Init();

  //<<AICUBE_USER_MAIN_CODE_BEGIN>>
  // 在此添加主函数中运行一次的用户代码
  //<<AICUBE_USER_MAIN_CODE_END>>

  while (1)
  {
    USBLIB_OUT_Done(); // 查询方式处理USB接收的数据

    //<<AICUBE_USER_MAIN_LOOP_BEGIN>>
    triggered = 0;
    for (ch = 8; ch <= 14; ch++)
    {
        adcValue = ADC_Convert(ch);

        if (adcValue < ADC_COMPARE_VALUE)
        {
            noteIndex = adcChannelToNote[ch]; // 使用映射表
            PWM_SetFreq(noteFreq[noteIndex]);
            isOpenPWM = 1;  // 只有检测到才开启
            triggered = 1;
            break; // 有一个触发就播放对应音调
        }
    }

    if (triggered != 1)
    {
        isOpenPWM = 0; // 没触发就关闭 PWM
        PWM_PIN = 0;   // 静音
    }
    delay_ms(50);
    //<<AICUBE_USER_MAIN_LOOP_END>>
  }
}

////////////////////////////////////////
// 系统初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void SYS_Init(void)
{
  EnableAccessXFR(); // 使能访问扩展XFR
  IAP_SetTimeBase(); // 设置IAP等待参数,产生1us时基

  //<<AICUBE_USER_PREINITIAL_CODE_BEGIN>>
  // 在此添加用户预初始化代码
  //<<AICUBE_USER_PREINITIAL_CODE_END>>

  P0M0 = 0x00;
  P0M1 = 0x00; // 初始化P0口为准双向口模式
  P1M0 = 0x00;
  P1M1 = 0x00; // 初始化P1口为准双向口模式
  P2M0 = 0x00;
  P2M1 = 0x00; // 初始化P2口为准双向口模式
  P3M0 = 0x00;
  P3M1 = 0x00; // 初始化P3口为准双向口模式
  P4M0 = 0x00;
  P4M1 = 0x00; // 初始化P4口为准双向口模式
  P5M0 = 0x00;
  P5M1 = 0x00; // 初始化P5口为准双向口模式
  P6M0 = 0x00;
  P6M1 = 0x00; // 初始化P6口为准双向口模式
  P7M0 = 0x00;
  P7M1 = 0x00; // 初始化P7口为准双向口模式

  TIMER0_Init(); // 定时器0初始化
  ADC_Init();    // ADC初始化
  delay_ms(1);
  USBLIB_Init(); // USB库初始化
  delay_ms(1);

  //<<AICUBE_USER_INITIAL_CODE_BEGIN>>
  // 在此添加用户初始化代码
  //<<AICUBE_USER_INITIAL_CODE_END>>

  EnableGlobalInt(); // 使能全局中断
}

////////////////////////////////////////
// 微秒延时函数
// 入口参数: us (设置延时的微秒值)
// 函数返回: 无
////////////////////////////////////////
void delay_us(uint16_t us)
{
  do
  {
    NOP(14); //(MAIN_Fosc + 500000) / 1000000 - 10
  } while (--us);
}

////////////////////////////////////////
// 毫秒延时函数
// 入口参数: ms (设置延时的毫秒值)
// 函数返回: 无
////////////////////////////////////////
void delay_ms(uint16_t ms)
{
  uint16_t i;

  do
  {
    i = MAIN_Fosc / 10000;
    while (--i)
      ;
  } while (--ms);
}

////////////////////////////////////////
// 定时器0初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void TIMER0_Init(void)
{
#define T0_RELOAD (65536 - (float)SYSCLK / 12 * 20 / 1000000) // 定时周期20微秒

  TIMER0_TimerMode();            // 设置定时器0为定时模式
  TIMER0_12TMode();              // 设置定时器0为12T模式
  TIMER0_Mode0();                // 设置定时器0为模式0 (16位自动重载模式)
  TIMER0_DisableGateINT0();      // 禁止定时器0门控
  TIMER0_SetIntPriority(3);      // 设置中断为最高优先级
  TIMER0_EnableInt();            // 使能定时器0中断
  TIMER0_SetReload16(T0_RELOAD); // 设置定时器0的16位重载值
  TIMER0_Run();                  // 定时器0开始运行

  //<<AICUBE_USER_TIMER0_INITIAL_BEGIN>>
  // 在此添加用户初始化代码
  //<<AICUBE_USER_TIMER0_INITIAL_END>>
}

////////////////////////////////////////
// ADC初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void ADC_Init(void)
{
  ADC_SetClockDivider(0); // 设置ADC时钟
  ADC_ResultRightAlign(); // 设置ADC结果右对齐(12位结果)
  ADC_SetRepeat2Times();  // ADC自动重复转换2次并取平均值

  ADC_SetCSSetupCycles(0);    // 设置ADC通道选择建立时间
  ADC_SetCSHoldCycles(1);     // 设置ADC通道选择保持时间
  ADC_SetSampleDutyCycles(9); // 设置ADC通道采样时间

  ADC_ActiveChannel(8); // 选择ADC通道
  ADC_Enable();         // 使能ADC功能

  //<<AICUBE_USER_ADC_INITIAL_BEGIN>>
  // 在此添加用户初始化代码
  //<<AICUBE_USER_ADC_INITIAL_END>>
}

////////////////////////////////////////
// 获取ADC转换结果函数
// 入口参数: ch (ADC通道选择)
// 函数返回: ADC转换结果
////////////////////////////////////////
uint16_t ADC_Convert(uint8_t ch)
{
  uint16_t res; // 定义保存ADC结果的变量

  ADC_ActiveChannel(ch); // 选择ADC通道
  ADC_Start();           // 开始ADC转换
  while (!ADC_CheckFlag())
    ;                     // 等待ADC转换完成
  ADC_ClearFlag();        // 清除ADC转换完成中断标志
  res = ADC_ReadResult(); // 读取ADC转换结果

  return res; // 返回ADC结果
}

////////////////////////////////////////
// USB库初始化函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void USBLIB_Init(void)
{
  usb_init();                 // 初始化USB模块
  USB_SetIntPriority(0);      // 设置中断为最低优先级
  set_usb_ispcmd("@STCISP#"); // 设置USB不停电下载命令

  //<<AICUBE_USER_USBLIB_INITIAL_BEGIN>>
  // 在此添加用户初始化代码
  //<<AICUBE_USER_USBLIB_INITIAL_END>>
}

////////////////////////////////////////
// 等待USB配置完成函数
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void USBLIB_WaitConfiged(void)
{
  while (DeviceState != DEVSTATE_CONFIGURED) // 等待USB完成配置
    WDT_Clear();                             // 清看门狗定时器 (防止硬件自动使能看门狗)
}

////////////////////////////////////////
// USB设备接收数据处理程序
// 入口参数: 无
// 函数返回: 无
// bUsbOutReady：USB设备接收数据标志位
// OutNumber：USB设备接收到的数据长度
// UsbOutBuffer：保存USB设备接收到的数据
////////////////////////////////////////
void USBLIB_OUT_Done(void)
{
  if (bUsbOutReady) // 查询是否有接收到USB主机发送数据
  {
    //<<AICUBE_USER_USBLIB_ISR_CODE1_BEGIN>>
    // 在此添加中断函数用户代码
    USB_SendData(UsbOutBuffer, OutNumber); // 原路返回, 用于测试
    // 在此处添加用户处理接收数据的代码
    //<<AICUBE_USER_USBLIB_ISR_CODE1_END>>
    usb_OUT_done(); // 当前包的数据处理完成,通知USB主机可以发送下一包数据
  }
}

////////////////////////////////////////
// 定时器0中断服务程序
// 入口参数: 无
// 函数返回: 无
////////////////////////////////////////
void TIMER0_ISR(void) interrupt TMR0_VECTOR
{
  //<<AICUBE_USER_TIMER0_ISR_CODE1_BEGIN>>
  if (isOpenPWM)
  {
    pwm_counter++;
    if (pwm_counter >= pwm_half_period_count)
    {
      pwm_counter = 0;
      PWM_PIN = !PWM_PIN; // 翻转引脚输出方波
    }
  }
  //<<AICUBE_USER_TIMER0_ISR_CODE1_END>>
}

//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
// 在此添加用户函数实现代码
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>
