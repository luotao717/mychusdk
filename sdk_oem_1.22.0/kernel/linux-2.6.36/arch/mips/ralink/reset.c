/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     reboot/reset setting for Ralink RT2880 solution
 *
 *  Copyright 2007 Ralink Inc. (bruce_chang@ralinktech.com.tw)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2007 Bruce Chang
 *
 * Initial Release
 *
 *
 *
 **************************************************************************
 */



#include <asm/reboot.h>
#include <asm/mach-ralink/generic.h>
#include <linux/pm.h>
#include <linux/delay.h>
#if defined(CONFIG_RALINK_MT7620)
#include "rt_timer.h"
#endif

static void mips_machine_restart(char *command);
static void mips_machine_halt(void);
static void mips_machine_power_off(void);

static void mips_machine_restart(char *command)
{
#if 0//defined(CONFIG_RALINK_MT7620)
    /* workaround: 7620 use watchdog to reset the system. */
    unsigned int result;
#if 0
#if defined(CONFIG_RALINK_MT7620)
    printk("CONFIG_RALINK_MT7620\n");
#endif
#if defined(CONFIG_RALINK_WATCHDOG)
    printk("CONFIG_RALINK_WATCHDOG\n");
#endif
#if defined(CONFIG_RALINK_TIMER_WDG)
    printk("CONFIG_RALINK_TIMER_WDG\n");
#endif
#if defined(CONFIG_RALINK_WATCHDOG_MODULE)
    printk("CONFIG_RALINK_WATCHDOG_MODULE\n");
#endif
#if defined(CONFIG_RALINK_TIMER_WDG_MODULE)
    printk("CONFIG_RALINK_TIMER_WDG_MODULE\n");
#endif
#endif
    printk("wdg reset\n");
	*(volatile unsigned int*)(TMR1LOAD) = 1 * (40000000/65536); //fixed at 40Mhz
    result=sysRegRead(TMR1CTL);
    result |= (1<<7);
    sysRegWrite(TMR1CTL,result);
    result=sysRegRead(GPIOMODE);
    /*
     * GPIOMODE[22:21] WDT_GPIO_MODE
     * 2'b00:Normal
     * 2'b01:REFCLK0
     * 2'b10:GPIO Mode
     */
    result &= ~(0x3<<21);
    result |= (0x0<<21);
    sysRegWrite(GPIOMODE,result);
#else
    printk("soft reset!\n");
	*(volatile unsigned int*)(SOFTRES_REG) = GORESET;
	*(volatile unsigned int*)(SOFTRES_REG) = 0;
#endif
}

static void mips_machine_halt(void)
{
	*(volatile unsigned int*)(SOFTRES_REG) = (0x1)<<26; // PCIERST
	mdelay(10);
	*(volatile unsigned int*)(SOFTRES_REG) = GORESET;
	*(volatile unsigned int*)(SOFTRES_REG) = 0;
}

static void mips_machine_power_off(void)
{
	*(volatile unsigned int*)(POWER_DIR_REG) = POWER_DIR_OUTPUT;
	*(volatile unsigned int*)(POWER_POL_REG) = 0;
	*(volatile unsigned int*)(POWEROFF_REG) = POWEROFF;
}


void mips_reboot_setup(void)
{
	_machine_restart = mips_machine_restart;
	_machine_halt = mips_machine_halt;
	//_machine_power_off = mips_machine_power_off;
	pm_power_off = mips_machine_power_off;
}
