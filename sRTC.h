/**
 * @file sRTC.h
 * @author silvio3105 (www.github.com/silvio3105)
 * @brief Simple RTC driver header file.
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


#ifndef _SRTC_H_
#define _SRTC_H_

/** \addtogroup sRTC
 * @{
 * Simple RTC driver.
*/

// STM32L051
#ifdef STM32L051xx

/** \defgroup STM32L051
 * @{
 * RTC for STM32L051 MCU.
*/

// ----- INCLUDE FILES
#include			<stm32l051xx.h>
#include			<system_stm32l0xx.h>


// ----- DEFINES
#define RTC_WPR_VAL1			0xCA /**< @brief Value1 for disabling write protection for RTC registers. */
#define RTC_WPR_VAL2			0x53 /**< @brief Value2 for disabling write protection for RTC registers. */
#define RTC_WPR_RST				0x00 /**< @brief Reset value for write protection register. */


// ----- ENUMS
/**
 * @brief Time format enum.
 * 
 */
enum sRTC_time_format_t : uint8_t {
	FORMAT_24H = 0, /**< @brief 24 hour time format. */
	AM_PM = 1 /**< @brief 12 hour AM/PM time format. */
};

/**
 * @brief Current time AM/PM enum.
 * 
 */
enum sRTC_ampm_t : uint8_t {
	AM = 0b0, /**< @brief Current time is AM. */
	PM = 0b1 /**< @brief Current time is PM. */
};

/**
 * @brief RTC mode enum.
 * 
 */
enum sRTC_mode_t : uint8_t {
	CONTINUE = 0, /**< @brief Continue with RTC clock when core is halted. */
	HALT = 1 /**< @brief Stop RTC when core is halted. */
};

/**
 * @brief RTC day enum.
 * 
 */
enum sRTC_day_t : uint8_t {
	MONDAY = 0b001,
	TUESDAY = 0b010,
	WEDNESDAY = 0b011,
	THURSDAY = 0b100,
	FRIDAY = 0b101,
	SATURDAY = 0b110,
	SUNDAY = 0b111
};

/**
 * @brief RTC wake up timer clock selection.
 * 
 */
enum sRTC_WUT_clock_t : uint8_t {
	RTC_16 = 0b000, /**< @brief RTC clock divided with 16. */
	RTC_8 = 0b001, /**< @brief RTC clock divided with 8. */
	RTC_4 = 0b010, /**< @brief RTC clock divided with 4. */
	RTC_2 = 0b011, /**< @brief RTC clock divided with 2. */
	CK_SPRE = 0b100 /**< @brief ~1Hz tick. */
};


// ----- STRUCTS
/**
 * @brief RTC time struct.
 * 
 * @note Time and date are stored as decimal values(NOT BCD!).
 */
struct sRTC_time_t {
	sRTC_day_t weekDay; /**< @brief Day of the week. See \ref sRTC_day_t */
	uint8_t day; /**< @brief Day stored as decimal value. */
	uint8_t month; /**< @brief Month stored as decimal value. */

	/**
	 * @brief Year stored as decimal value.
	 * 
	 * @note Stored as desired year - 2000.
	 */
	uint8_t year;
	uint8_t hour; /**< @brief Hour stored as decimal value. */
	uint8_t minute; /**< @brief Minute stored as decimal value. */
	uint8_t second; /**< @brief Second stored as decimal value. */
	sRTC_ampm_t ampm = sRTC_ampm_t::AM; /**< @brief Current time AM/PM. Optional. See \ref sRTC_ampm_t */
};


// ----- CLASSES
class sRTC {
	// PUBLIC STUFF
	public:
	// CONSTRUCTOR AND DECONSTRUCTOR DECLARATIONS
	/**
	 * @brief Object constructor.
	 * 
	 * @param rtcHandle Pointer to RTC peripheral.
	 * @return No return value.
	 */
	sRTC(RTC_TypeDef* rtcHandle);

	/**
	 * @brief Object deconstructor.
	 * 
	 * @return No return value.
	 */
	~sRTC(void);

	// METHOD DECLARATIONS
	/**
	 * @brief Init RTC.
	 * 
	 * @param format Time format. See \ref sRTC_time_format_t
	 * @param time Pointer to time struct. Optional. Set to \c nullptr init RTC without time and date.
	 * @param mode Working mode when core is halted. Optional. See \ref sRTC_mode_t
	 */
	void init(sRTC_time_format_t format, sRTC_time_t* time = nullptr, sRTC_mode_t mode = sRTC_mode_t::CONTINUE);

	/**
	 * @brief Set time and date.
	 * 
	 * @param day Day as decimal value.
	 * @param month Month as decimal value.
	 * @param year Desired year - 2000 as decimal value.
	 * @param weekDay Day of week. See \ref sRTC_day_t
	 * @param hour Hour as decimal value.
	 * @param minute Minute as decimal value.
	 * @param second Second as decimal value.
	 * @param ampm Select AM or PM for time. Not necessary if time format is \ref sRTC_time_format_t::FORMAT_24H
	 */
	void set(uint8_t day, uint8_t month, uint8_t year, sRTC_day_t weekDay, uint8_t hour, uint8_t minute, uint8_t second, sRTC_ampm_t ampm = sRTC_ampm_t::AM);

	/**
	 * @brief Set time and date.
	 * 
	 * @param time Reference to struct with time and date in decimal values.
	 * @return No return value.
	 */
	inline void set(sRTC_time_t& time)
	{
		// Pass values to main set method
		set(time.day, time.month, time.year, time.weekDay,
			time.hour, time.minute, time.second, time.ampm);	
	}

	/**
	 * @brief Get time and date.
	 * 
	 * @param day Reference to variable for day as decimal value.
	 * @param month Reference to variable for month as decimal value.
	 * @param year Reference to variable for year - 2000 as decimal value.
	 * @param weekDay Reference to variable for day of the week. See \ref sRTC_day_t
	 * @param hour Reference to variable for hour as decimal value.
	 * @param minute Reference to variable for minute as decimal value.
	 * @param second Reference to variable for second as decimal value.
	 * @param ampm Reference to variable for AM/PM. Used only if time format is \ref sRTC_time_format_t::FORMAT_24H
	 */
	void get(uint8_t& day, uint8_t& month, uint8_t& year, sRTC_day_t& weekDay, uint8_t& hour, uint8_t& minute, uint8_t& second, sRTC_ampm_t& ampm) const;

	/**
	 * @brief Get time and date.
	 * 
	 * @param time Reference to struct where time and date will be stored as decimal values.
	 * @return No return value.
	 */
	inline void get(sRTC_time_t& time) const
	{
		// Pass to main get method
		get(time.day, time.month, time.year, time.weekDay, time.hour, time.minute, time.second, time.ampm);
	}

	/**
	 * @brief Enable wake up timer.
	 * 
	 * @param clock Input clock prescaler. See \ref sRTC_WUT_clock_t
	 * @param reloadValue Wake up timer reload value.
	 * @return No return value.
	 */
	void enableWakeup(sRTC_WUT_clock_t clock, uint16_t reloadValue);

	/**
	 * @brief Disable wake up timer.
	 * 
	 * @return No return value.
	 */
	void disableWakeup(void);

	/**
	 * @brief Is RTC configured.
	 * 
	 * @return \c 0 if RTC is not configured.
	 * @return\c 1 if RTC is configured. 
	 */
	inline uint8_t isSet(void) const
	{
		// Return INITS bit
		return (handle->ISR & RTC_ISR_INITS);
	}


	// PRIVATE STUFF
	private:
	// VARIABLES
	RTC_TypeDef* handle = nullptr; /**< @brief RTC handle. */

	// METHOD DECLARATIONS
	/**
	 * @brief Set date.
	 * 
	 * @param day Day in decimal value.
	 * @param month Month in decimal value.
	 * @param year Year - 2000 in decimal value.
	 * @param weekDay Day of the week. See \ref sRTC_day_t
	 * @return No return value.
	 */
	void setDate(uint8_t day, uint8_t month, uint8_t year, sRTC_day_t weekDay);

	/**
	 * @brief Set time.
	 * 
	 * @param hour Hour in decimal value.
	 * @param minute Minute in decimal value.
	 * @param second Second in decimal value.
	 * @param ampm Set AM or PM. See \ref sRTC_ampm_t
	 * @return No return value.
	 */
	void setTime(uint8_t hour, uint8_t minute, uint8_t second, sRTC_ampm_t ampm);

	/**
	 * @brief Get day from RTC.
	 * 
	 * @param value Input date register.
	 * @return Day in decimal.
	 */
	uint8_t getDay(uint32_t value) const;

	/**
	 * @brief Get month from RTC.
	 * 
	 * @param value Input date register.
	 * @return Month in decimal. 
	 */
	uint8_t getMonth(uint32_t value) const;

	/**
	 * @brief Get year from RTC.
	 * 
	 * @param value Input date register.
	 * @return Year - 2000 in decimal.
	 */
	uint8_t getYear(uint32_t value) const;

	/**
	 * @brief Get hour from RTC.
	 * 
	 * @param value Input time register.
	 * @return Hour in decimal.
	 */
	uint8_t getHour(uint32_t value) const;

	/**
	 * @brief Get minute from RTC.
	 * 
	 * @param value Input time register.
	 * @return Minute in decimal.
	 */
	uint8_t getMinute(uint32_t value) const;

	/**
	 * @brief Get second from RTC.
	 * 
	 * @param value Input time register.
	 * @return Second in decimal. 
	 */
	uint8_t getSecond(uint32_t value) const;


	/**
	 * @brief Get day in week from RTC.
	 * 
	 * @param value Input date register.
	 * @return Day in week. See \ref sRTC_day_t
	 */
	inline sRTC_day_t getWeekDay(uint32_t value) const
	{
		return (sRTC_day_t)((value & RTC_DR_WDU) >> RTC_DR_WDU_Pos);
	}

	/**
	 * @brief Get AM/PM tag from RTC.
	 * 
	 * @param value Input time register.
	 * @return AM/PM tag. See \ref sRTC_ampm_t
	 */
	inline sRTC_ampm_t getAMPM(uint32_t value) const
	{
		return (sRTC_ampm_t)((value & RTC_TR_PM) >> RTC_TR_PM_Pos);
	}	

	/**
	 * @brief Disable write protection for RTC registers.
	 * 
	 * @return No return value.
	 */
	inline void disableWP(void)
	{
		// Write required values to disable write protection
		handle->WPR = RTC_WPR_VAL1;
		handle->WPR = RTC_WPR_VAL2;
	}

	/**
	 * @brief Enable write protection for RTC registers.
	 * 
	 * @return No return value.
	 */
	inline void enableWP(void)
	{
		// Enable write protection
		handle->WPR = RTC_WPR_RST;
	}

	/**
	 * @brief Enable init phase.
	 * 
	 * @return No return value.
	 */
	inline void enableInit(void)
	{
		// Enable RTC init
		handle->ISR |= RTC_ISR_INIT;

		// Wait for RTC to stop fir init phase
		while (!(handle->ISR & RTC_ISR_INITF));
	}

	/**
	 * @brief Disable init phase.
	 * 
	 * @return No return value.
	 */
	inline void disableInit(void)
	{
		// Disable RTC init
		handle->ISR &= ~RTC_ISR_INIT;
	}

	/**
	 * @brief Disable RTC backup domain write protection.
	 * 
	 * @return No return value.
	 */
	inline void disableBackupWP(void) const
	{
		// Set backup domain bit in PWR
		PWR->CR |= PWR_CR_DBP;
	}

	/**
	 * @brief Enable RTC backup domain write protection.
	 * 
	 * @return No return value.
	 */
	inline void enableBackupWP(void) const
	{
		// Clear backup domain bit in PWR
		PWR->CR &= ~PWR_CR_DBP;	
	}
};


/**@}*/

#endif // STM32L051xx

/**@}*/

#endif // _SRTC_H_

// END WITH NEW LINE
