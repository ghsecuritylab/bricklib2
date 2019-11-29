/* bricklib2
 * Copyright (C) 2017 Olaf Lüke <olaf@tinkerforge.com>
 *
 * ccu8_pwm.c: Very simple XMC1X00 CCU8 PWM configuration
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
**/

#include "ccu8_pwm.h"

#include "configs/config.h"

XMC_CCU8_SLICE_t *const slice[4] = {
	CCU80_CC80,
	CCU80_CC81,
	CCU80_CC82,
	CCU80_CC83,
};

// Changes period. This does not do shadow transfer, always call set_duty_cycle afterwards!
void ccu8_pwm_set_period(const uint8_t ccu8_slice_number, const uint16_t period_value) {
    XMC_CCU8_SLICE_SetTimerPeriodMatch(slice[ccu8_slice_number], period_value);
}

// Compare value is a value from 0 to period_value (^= 0 to 100% duty cycle)
void ccu8_pwm_set_duty_cycle(const uint8_t ccu8_slice_number, const uint16_t compare_value) {
	XMC_CCU8_SLICE_SetTimerCompareMatch(slice[ccu8_slice_number], XMC_CCU8_SLICE_COMPARE_CHANNEL_1, compare_value);
    XMC_CCU8_EnableShadowTransfer(CCU80, (XMC_CCU8_SHADOW_TRANSFER_SLICE_0 << (ccu8_slice_number*4)) |
    		                             (XMC_CCU8_SHADOW_TRANSFER_PRESCALER_SLICE_0 << (ccu8_slice_number*4)));
}

// Period value is the amount of clock cycles per period
void ccu8_pwm_init(XMC_GPIO_PORT_t *const port, const uint8_t pin, const uint8_t ccu8_slice_number, const uint16_t period_value) {
	const XMC_CCU8_SLICE_COMPARE_CONFIG_t compare_config = {
		.timer_mode          = XMC_CCU8_SLICE_TIMER_COUNT_MODE_EA,
		.monoshot            = false,
		.shadow_xfer_clear   = 0,
		.dither_timer_period = 0,
		.dither_duty_cycle   = 0,
		.prescaler_mode      = XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,
#ifdef CCU8_PWM_PRESCALER
		.prescaler_initval   = CCU8_PWM_PRESCALER,
#else
		.prescaler_initval   = 0,
#endif
		.float_limit         = XMC_CCU8_SLICE_PRESCALER_32768,
		.float_limit         = 0,
		.dither_limit        = 0,
		.passive_level_out0  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out1  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out2  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.passive_level_out3  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
		.asymmetric_pwm      = false,
		.timer_concatenation = 0
	};

	const XMC_GPIO_CONFIG_t gpio_out_config	= {
#ifdef CCU8_PWM_PUSH_PULL_ALT
		.mode                = CCU8_PWM_PUSH_PULL_ALT,
#else
		.mode                = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT5,
#endif
		.input_hysteresis    = XMC_GPIO_INPUT_HYSTERESIS_STANDARD,
		.output_level        = XMC_GPIO_OUTPUT_LEVEL_LOW,
	};

    XMC_CCU8_Init(CCU80, XMC_CCU8_SLICE_MCMS_ACTION_TRANSFER_PR_CR);
    XMC_CCU8_StartPrescaler(CCU80);
    XMC_CCU8_SLICE_CompareInit(slice[ccu8_slice_number], &compare_config);

    // Set the period and compare register values
    XMC_CCU8_SLICE_SetTimerPeriodMatch(slice[ccu8_slice_number], period_value);
    XMC_CCU8_SLICE_SetTimerCompareMatch(slice[ccu8_slice_number], XMC_CCU8_SLICE_COMPARE_CHANNEL_1, 0);

    XMC_CCU8_EnableShadowTransfer(CCU80, (XMC_CCU8_SHADOW_TRANSFER_SLICE_0 << (ccu8_slice_number*4)) |
    		                             (XMC_CCU8_SHADOW_TRANSFER_PRESCALER_SLICE_0 << (ccu8_slice_number*4)));

    XMC_GPIO_Init(port, pin, &gpio_out_config);

    XMC_CCU8_EnableClock(CCU80, ccu8_slice_number);
    XMC_CCU8_SLICE_StartTimer(slice[ccu8_slice_number]);
}