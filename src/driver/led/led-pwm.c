/*
 * driver/led/led-pwm.c
 *
 * Copyright(c) 2007-2022 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <xboot.h>
#include <pwm/pwm.h>
#include <regulator/regulator.h>
#include <led/led.h>

/*
 * PWM LED - LED Driver Using Pulse Width Modulation
 *
 * Required properties:
 * - pwm-name: led attached pwm
 *
 * Optional properties:
 * - regulator-name: supply regulator
 * - pwm-period-ns: pwm period in ns
 * - pwm-polarity: pwm polarity
 * - gamma-correction: gamma correction
 * - default-brightness: led default brightness
 *
 * Example:
 *   "led-pwm@0": {
 *       "pwm-name": "pwm.0",
 *       "regulator-name": "led-power",
 *       "pwm-period-ns": 10000000,
 *       "pwm-polarity": false,
 *       "gamma-correction": 2.2,
 *       "default-brightness": 0
 *   }
 */

struct led_pwm_pdata_t {
	struct pwm_t * pwm;
	char * regulator;
	int period;
	int polarity;
	int gamma_lut[1000];
	int brightness;
};

static void led_pwm_set_brightness(struct led_pwm_pdata_t * pdat, int brightness)
{
	int value = pdat->gamma_lut[brightness];
	if(value > 0)
	{
		int duty = value * pdat->period / 1000;
		pwm_config(pdat->pwm, duty, pdat->period, pdat->polarity);
		pwm_enable(pdat->pwm);
	}
	else
	{
		pwm_disable(pdat->pwm);
	}
}

static void led_pwm_set(struct led_t * led, int brightness)
{
	struct led_pwm_pdata_t * pdat = (struct led_pwm_pdata_t *)led->priv;

	if(pdat->brightness != brightness)
	{
		if(pdat->regulator)
		{
			if(brightness <= 0)
				regulator_disable(pdat->regulator);
			else if(pdat->brightness <= 0)
				regulator_enable(pdat->regulator);
		}
		led_pwm_set_brightness(pdat, brightness);
		pdat->brightness = brightness;
	}
}

static int led_pwm_get(struct led_t * led)
{
	struct led_pwm_pdata_t * pdat = (struct led_pwm_pdata_t *)led->priv;
	return pdat->brightness;
}

static struct device_t * led_pwm_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct led_pwm_pdata_t * pdat;
	struct pwm_t * pwm;
	struct led_t * led;
	struct device_t * dev;

	if(!(pwm = search_pwm(dt_read_string(n, "pwm-name", NULL))))
		return NULL;

	pdat = malloc(sizeof(struct led_pwm_pdata_t));
	if(!pdat)
		return NULL;

	led = malloc(sizeof(struct led_t));
	if(!led)
	{
		free(pdat);
		return NULL;
	}

	pdat->pwm = pwm;
	pdat->regulator = strdup(dt_read_string(n, "regulator-name", NULL));
	pdat->period = dt_read_int(n, "pwm-period-ns", 1000 * 1000);
	pdat->polarity = dt_read_bool(n, "pwm-polarity", 1);
	float gamma = dt_read_double(n, "gamma-correction", 2.2);
	for(int i = 0; i < 1000; i++)
		pdat->gamma_lut[i] = (int)(roundf(powf(i / 1000.0f, gamma) * 1000.0f));
	pdat->brightness = -1;

	led->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	led->set = led_pwm_set;
	led->get = led_pwm_get;
	led->priv = pdat;

	led_pwm_set(led, dt_read_int(n, "default-brightness", 0));

	if(!(dev = register_led(led, drv)))
	{
		if(pdat->regulator)
			free(pdat->regulator);
		free_device_name(led->name);
		free(led->priv);
		free(led);
		return NULL;
	}
	return dev;
}

static void led_pwm_remove(struct device_t * dev)
{
	struct led_t * led = (struct led_t *)dev->priv;
	struct led_pwm_pdata_t * pdat = (struct led_pwm_pdata_t *)led->priv;

	if(led)
	{
		unregister_led(led);
		if(pdat->regulator)
			free(pdat->regulator);
		free_device_name(led->name);
		free(led->priv);
		free(led);
	}
}

static void led_pwm_suspend(struct device_t * dev)
{
	struct led_t * led = (struct led_t *)dev->priv;
	struct led_pwm_pdata_t * pdat = (struct led_pwm_pdata_t *)led->priv;
	if(pdat->regulator)
	{
		if(pdat->brightness > 0)
			regulator_disable(pdat->regulator);
	}
	led_pwm_set_brightness(pdat, 0);
}

static void led_pwm_resume(struct device_t * dev)
{
	struct led_t * led = (struct led_t *)dev->priv;
	struct led_pwm_pdata_t * pdat = (struct led_pwm_pdata_t *)led->priv;
	if(pdat->regulator)
	{
		if(pdat->brightness > 0)
			regulator_enable(pdat->regulator);
	}
	led_pwm_set_brightness(pdat, pdat->brightness);
}

static struct driver_t led_pwm = {
	.name		= "led-pwm",
	.probe		= led_pwm_probe,
	.remove		= led_pwm_remove,
	.suspend	= led_pwm_suspend,
	.resume		= led_pwm_resume,
};

static __init void led_pwm_driver_init(void)
{
	register_driver(&led_pwm);
}

static __exit void led_pwm_driver_exit(void)
{
	unregister_driver(&led_pwm);
}

driver_initcall(led_pwm_driver_init);
driver_exitcall(led_pwm_driver_exit);
