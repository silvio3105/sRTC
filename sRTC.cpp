/**
 * @file sRTC.cpp
 * @author silvio3105 (www.github.com/silvio3105)
 * @brief Simple RTC driver translation unit.
 * 
 * @copyright Copyright (c) 2023, silvio3105
 * 
 */

/*
License

Copyright (c) 2023, silvio3105 (www.github.com/silvio3105)

Access and use of this Project and its contents are granted free of charge to any Person.
The Person is allowed to copy, modify and use The Project and its contents only for non-commercial use.
Commercial use of this Project and its contents is prohibited.
Modifying this License and/or sublicensing is prohibited.

THE PROJECT AND ITS CONTENT ARE PROVIDED "AS IS" WITH ALL FAULTS AND WITHOUT EXPRESSED OR IMPLIED WARRANTY.
THE AUTHOR KEEPS ALL RIGHTS TO CHANGE OR REMOVE THE CONTENTS OF THIS PROJECT WITHOUT PREVIOUS NOTICE.
THE AUTHOR IS NOT RESPONSIBLE FOR DAMAGE OF ANY KIND OR LIABILITY CAUSED BY USING THE CONTENTS OF THIS PROJECT.

This License shall be included in all methodal textual files.
*/


// ----- INCLUDE FILES
#include			"sRTC.h"


// ----- STATIC FUNCTION DECLARATIONS
/**
 * @brief Convert decimal value to BCD value.
 * 
 * @param value Input 8-bit decimal value.
 * @return BCD value.
 */
static uint8_t dec2BCD(uint8_t value);

/**
 * @brief Convert BCD value to decimal value.
 * 
 * @param value Input 8-bit BCD value.
 * @return Decimal value.
 */
static uint8_t BCD2Dec(uint8_t value);


// ----- METHOD DEFINITIONS
sRTC::sRTC(RTC_TypeDef* rtcHandle)
{
	// Set RTC handle
	handle = rtcHandle;
}

sRTC::~sRTC(void)
{
	// Reset handler
	handle = nullptr;
}

void sRTC::init(sRTC_time_format_t format, sRTC_time_t* time, sRTC_mode_t mode)
{
	// Enable RTC clock
	RCC->CSR |= RCC_CSR_RTCEN;

	// Disable RTC backup register write protection
	disableBackupWP();

	// Disable write protection
	disableWP();

	// Enable init phase
	enableInit();	

	// Set RTC A-prescaler to 127 and S-prescaler to 255(default values)
	handle->PRER = 0x007F00FF;

	// Set time format and disable shadow bypass
	handle->CR = 0x00000000;
	handle->CR |= (format << RTC_CR_FMT_Pos) | (0 << RTC_CR_BYPSHAD_Pos);

	// If time pointer is not NULL
	if (time)
	{
		// Set RTC time
		setDate(time->day, time->month, time->year, time->weekDay);
		setTime(time->hour, time->minute, time->second, time->ampm);	
	}

	// Enable write protection
	enableWP();	

	// Disable init phase
	disableInit();

	// Enable RTC backup register write protection
	enableBackupWP();	

	// Enable RTC EXTI line
	EXTI->IMR |= EXTI_IMR_IM20;
	EXTI->RTSR |= EXTI_IMR_IM20;

	// Enable RTC IRQ
	NVIC_SetPriority(RTC_IRQn, 0);
	NVIC_EnableIRQ(RTC_IRQn);	

	// Set halt mode when in DEBUG mode
	if (mode == sRTC_mode_t::HALT) DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_RTC_STOP;	
}

void sRTC::set(uint8_t day, uint8_t month, uint8_t year, sRTC_day_t weekDay, uint8_t hour, uint8_t minute, uint8_t second, sRTC_ampm_t ampm)
{
	// Disable RTC backup register write protection
	disableBackupWP();

	// Disable write protection
	disableWP();

	// Enable init phase
	enableInit();

	// Set RTC time
	setDate(day, month, year, weekDay);
	setTime(hour, minute, second, ampm);

	// Enable write protection
	enableWP();	

	// Disable init phase
	disableInit();

	// Enable RTC backup register write protection
	enableBackupWP();
}

void sRTC::get(uint8_t& day, uint8_t& month, uint8_t& year, sRTC_day_t& weekDay, uint8_t& hour, uint8_t& minute, uint8_t& second, sRTC_ampm_t& ampm) const
{
	uint32_t value = handle->TR;

	// Get values from time register
	hour = getHour(value);
	minute = getMinute(value);
	second = getSecond(value);
	ampm = (sRTC_ampm_t)getAMPM(value);

	// Get values from date register
	value = handle->DR;
	weekDay = (sRTC_day_t)getWeekDay(value);
	day = getDay(value);
	month = getMonth(value);
	year = getYear(value);
}

void sRTC::wakeupStart(uint16_t reloadValue)
{
	// Disable RTC backup register write protection
	disableBackupWP();

	// Disable write protection
	disableWP();

	// Set wake up reload value
	handle->WUTR = reloadValue - 1;

	// Clear interrupt flag
	handle->ISR &= ~RTC_ISR_WUTF;

	// Enable wake up timer
	handle->CR |= RTC_CR_WUTE;

	// Enable RTC write protection
	enableWP();

	// Enable RTC backup register write protection
	enableBackupWP();	
}

void sRTC::wakeupEnable(sRTC_WUT_clock_t clock)
{
	// Disable RTC backup register write protection
	disableBackupWP();

	// Disable write protection
	disableWP();

	// Disable wake up timer
	handle->CR &= ~(RTC_CR_WUTE);

	// Wait for write flag to set
	while (!(handle->ISR & RTC_ISR_WUTWF));

	// Clear wake up write interrupt flag
	handle->ISR &= ~RTC_ISR_WUTWF;

	// Clear wake up timer clock prescaler
	handle->CR &= ~RTC_CR_WUCKSEL;

	// Set wake up timer clock prescaler
	handle->CR |= clock;

	// Clear interrupt flag
	handle->ISR &= ~RTC_ISR_WUTF;

	// Enable wake up timer interrupt
	handle->CR |= RTC_CR_WUTIE;

	// Enable wake up from RTC
	PWR->CSR |= PWR_CSR_WUF;

	// Enable RTC write protection
	enableWP();

	// Enable RTC backup register write protection
	enableBackupWP();		
}

void sRTC::wakeupDisable(void)
{
	// Disable RTC backup register write protection
	disableBackupWP();

	// Disable write protection
	disableWP();

	// Disable wake up timer and its interrupt
	handle->CR &= ~(RTC_CR_WUTE | RTC_CR_WUTIE);

	// Reset wake up timer counter
	handle->WUTR = 0;

	// Enable RTC write protection
	enableWP();

	// Enable RTC backup register write protection
	enableBackupWP();	
}

void sRTC::calibrate(sRTC_cal_dir_t direction, uint16_t value, sRTC_cal_cycle_t cycle)
{
	uint8_t calp = 0;
	uint32_t calCycle = 0;

	// Disable RTC backup register write protection
	disableBackupWP();

	// Disable write protection
	disableWP();

	// Enable init phase
	enableInit();	

	// Set CALP bit
	if (direction == sRTC_cal_dir_t::RTC_CAL_POSITIVE) calp = 1;

	// Set cycle bit and limit first two bits
	if (cycle == sRTC_cal_cycle_t::RTC_CAL_16S)
	{
		calCycle = RTC_CALR_CALW16;
		value &= ~1;
	}
	else if (cycle == sRTC_cal_cycle_t::RTC_CAL_8S)
	{
		calCycle = RTC_CALR_CALW8;
		value &= ~3;
	}

	// Wait until it is allowed to do calibration
	while (handle->ISR & RTC_ISR_RECALPF);

	// Set CALR register
	handle->CALR = 0 | (calp << RTC_CALR_CALP_Pos) | value | calCycle;

	// Disable init phase
	disableInit();

	// Enable RTC write protection
	enableWP();

	// Enable RTC backup register write protection
	enableBackupWP();		
}


// ----- PRIVATE METHOD DEFINITIONS
void sRTC::setDate(uint8_t day, uint8_t month, uint8_t year, sRTC_day_t weekDay)
{
	// Reset date register to default value
	handle->DR = 0x00002101;

	// Convert input values to BCD
	day = dec2BCD(day);
	month = dec2BCD(month);
	year = dec2BCD(year);

	// Set RTC date
	handle->DR = (weekDay << RTC_DR_WDU_Pos) | (day << RTC_DR_DU_Pos) | (month << RTC_DR_MU_Pos) | (year << RTC_DR_YU_Pos);
}

void sRTC::setTime(uint8_t hour, uint8_t minute, uint8_t second, sRTC_ampm_t ampm)
{
	// Reset time register to default value
	handle->TR = 0x00000000;

	// Convert input values to BCD
	hour = dec2BCD(hour);
	minute = dec2BCD(minute);
	second = dec2BCD(second);

	// Set RTC time
	handle->TR = (ampm << RTC_TR_PM_Pos) | (hour << RTC_TR_HU_Pos) | (minute << RTC_TR_MNU_Pos) | (second << RTC_TR_SU_Pos);
}

uint8_t sRTC::getDay(uint32_t value) const
{
	return BCD2Dec((value & (RTC_DR_DT | RTC_DR_DU)) >> RTC_DR_DU_Pos);
}

uint8_t sRTC::getMonth(uint32_t value) const
{
	return BCD2Dec((value & (RTC_DR_MT | RTC_DR_MU)) >> RTC_DR_MU_Pos);
}

uint8_t sRTC::getYear(uint32_t value) const
{
	return BCD2Dec((value & (RTC_DR_YT | RTC_DR_YU)) >> RTC_DR_YU_Pos);
}

uint8_t sRTC::getHour(uint32_t value) const
{
	return BCD2Dec((value & (RTC_TR_HU | RTC_TR_HT)) >> RTC_TR_HU_Pos);
}

uint8_t sRTC::getMinute(uint32_t value) const
{
	return BCD2Dec((value & (RTC_TR_MNU | RTC_TR_MNT)) >> RTC_TR_MNU_Pos);
}

uint8_t sRTC::getSecond(uint32_t value) const
{
	return BCD2Dec(value & (RTC_TR_SU | RTC_TR_ST));
}


// ----- STATIC FUNCTION DEFINITIONS
static uint8_t dec2BCD(uint8_t value)
{
	// Return decimal value converted to BCD
	return (((value / 10) << 4) + (value % 10));
}

static uint8_t BCD2Dec(uint8_t value)
{
	// Return BCD value converted to decimal
	return (((value >> 4) * 10) + (value & 0x0F));
}


// END WITH NEW LINE
