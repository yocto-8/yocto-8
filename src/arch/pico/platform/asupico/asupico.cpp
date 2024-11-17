#include "video/dwoqspi.hpp"
#include <array>
#include <cstdint>
#include <hardware/irq.h>
#include <hardware/pwm.h>
#include <hardware/regs/intctrl.h>
#include <pico/stdio.h>
#include <platform/asupico/asupico.hpp>

#include <cstdio>
#include <ff.h>
#include <hardware/clocks.h>
#include <hardware/structs/pads_qspi.h>
#include <hardware/structs/qmi.h>
#include <hardware/structs/xip.h>
#include <hardware/sync.h>
#include <hardware/vreg.h>
#include <pico/stdlib.h>

#include <hardwarestate.hpp>

namespace arch::pico::platform::asupico {

struct GlobalConstants {
	std::size_t spi_video_clock;
	video::DWO::Config dwo;
	struct {
		int dir_left, dir_up, dir_right, dir_down, act_o, act_x;
	} button_pinout; // TODO: unify across arch
	int led_pin;
	int psram_cs;
	vreg_voltage core_voltage;
	unsigned core_frequency_khz;
};

static const GlobalConstants config = {.spi_video_clock = 50'000'000,
                                       .dwo = {.spi = spi0,
                                               .pio = pio0,
                                               .pio_sm = 0,
                                               .pinout =
                                                   {
													   .sclk = 2,
													   .cs = 5,
													   .te = 11,
													   .sio0 = 7,
													   .qsi1 = 8,
													   .qsi2 = 9,
													   .qsi3 = 10,
													   .rst = 14,
													   .pwr_en = 15,
												   }},
                                       .button_pinout = {.dir_left = 16,
                                                         .dir_up = 17,
                                                         .dir_right = 18,
                                                         .dir_down = 19,
                                                         .act_o = 20,
                                                         .act_x = 21},
                                       .led_pin = PICO_DEFAULT_LED_PIN,
                                       .psram_cs =
                                           PIMORONI_PICO_PLUS2_PSRAM_CS_PIN,
                                       .core_voltage = VREG_VOLTAGE_1_25,
                                       .core_frequency_khz = 351000};

namespace state {
// video::SSD1351 ssd1351;
video::DWO dwo;
std::array<io::PushButton, 6> buttons;
// FIXME: buffer not in PSRAM because we cannot access(?) PSRAM during flashing
/*[[gnu::section(Y8_PSRAM_SECTION)]]*/ FATFS flash_fatfs;
} // namespace state

[[gnu::section(Y8_CORE1_SRAM_SECTION), gnu::noinline]] void
gpio_irq_handler(uint gpio, uint32_t event_mask) {
	if (gpio == config.dwo.pinout.te &&
	    (event_mask & GPIO_IRQ_EDGE_RISE) != 0) {
		state::dwo.start_scanout();
	}
}

void __no_inline_not_in_flash_func(init_flash_frequency)() {
	uint32_t interrupt_state = save_and_disable_interrupts();

	// pad configuration; see boot2_w25q080.S
	//     ldr r3, =PADS_QSPI_BASE
	//     movs r0, INIT_PAD_SCLK
	//     str r0, [r3, #PADS_QSPI_GPIO_QSPI_SCLK_OFFSET]
	//     // SDx: disable input Schmitt to reduce delay
	//     adds r3, #REG_ALIAS_CLR_BITS
	//     movs r0, #PADS_QSPI_GPIO_QSPI_SD0_SCHMITT_BITS
	//     str r0, [r3, #PADS_QSPI_GPIO_QSPI_SD0_OFFSET]
	//     str r0, [r3, #PADS_QSPI_GPIO_QSPI_SD1_OFFSET]
	//     str r0, [r3, #PADS_QSPI_GPIO_QSPI_SD2_OFFSET]
	//     str r0, [r3, #PADS_QSPI_GPIO_QSPI_SD3_OFFSET]

	// 0x00000100 [8]     ISO          (1) Pad isolation control
	// 0x00000080 [7]     OD           (0) Output disable
	// 0x00000040 [6]     IE           (1) Input enable
	// 0x00000030 [5:4]   DRIVE        (0x1) Drive strength
	// 0x00000008 [3]     PUE          (0) Pull up enable
	// 0x00000004 [2]     PDE          (1) Pull down enable
	// 0x00000002 [1]     SCHMITT      (1) Enable schmitt trigger
	// 0x00000001 [0]     SLEWFAST     (0) Slew rate control
	pads_qspi_hw->io[0] = 2 << PADS_QSPI_GPIO_QSPI_SCLK_DRIVE_LSB |
	                      PADS_QSPI_GPIO_QSPI_SCLK_SLEWFAST_BITS;
	pads_qspi_hw->io[1] &= ~PADS_QSPI_GPIO_QSPI_SD0_SCHMITT_BITS;
	pads_qspi_hw->io[2] &= ~PADS_QSPI_GPIO_QSPI_SD0_SCHMITT_BITS;
	pads_qspi_hw->io[3] &= ~PADS_QSPI_GPIO_QSPI_SD0_SCHMITT_BITS;
	pads_qspi_hw->io[4] &= ~PADS_QSPI_GPIO_QSPI_SD0_SCHMITT_BITS;

	qmi_hw->m[0].timing = 1 << QMI_M0_TIMING_COOLDOWN_LSB |
	                      2 << QMI_M0_TIMING_RXDELAY_LSB |
	                      3 << QMI_M0_TIMING_CLKDIV_LSB;
	qmi_hw->m[0].rcmd =
		0xEB << QMI_M0_RCMD_PREFIX_LSB /* | 0xA0 << QMI_M0_RCMD_SUFFIX_LSB*/;
	qmi_hw->m[0].rfmt =
		QMI_M0_RFMT_PREFIX_WIDTH_VALUE_S << QMI_M0_RFMT_PREFIX_WIDTH_LSB |
		QMI_M0_RFMT_ADDR_WIDTH_VALUE_Q << QMI_M0_RFMT_ADDR_WIDTH_LSB |
		QMI_M0_RFMT_SUFFIX_WIDTH_VALUE_Q << QMI_M0_RFMT_SUFFIX_WIDTH_LSB |
		QMI_M0_RFMT_DUMMY_WIDTH_VALUE_Q << QMI_M0_RFMT_DUMMY_WIDTH_LSB |
		QMI_M0_RFMT_DATA_WIDTH_VALUE_Q << QMI_M0_RFMT_DATA_WIDTH_LSB |
		QMI_M0_RFMT_PREFIX_LEN_VALUE_8 << QMI_M0_RFMT_PREFIX_LEN_LSB |
		QMI_M0_RFMT_SUFFIX_LEN_VALUE_8 << QMI_M0_RFMT_SUFFIX_LEN_LSB |
		4 << QMI_M0_RFMT_DUMMY_LEN_LSB;

	// dummy read
	*reinterpret_cast<volatile char *>(XIP_NOCACHE_NOALLOC_BASE);

	restore_interrupts(interrupt_state);
}

void init_default_frequency() {
	vreg_set_voltage(config.core_voltage);
	set_sys_clock_khz(config.core_frequency_khz, true);

	// clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
	// 351 * MHZ, 351 * MHZ);

	init_stdio();
}

void init_stdio() { stdio_init_all(); }

void init_basic_gpio() {
	gpio_set_function(config.led_pin, GPIO_FUNC_PWM);
	pwm_set_enabled(pwm_gpio_to_slice_num(config.led_pin), true);
	pwm_set_wrap(pwm_gpio_to_slice_num(config.led_pin), 65535);

	state::buttons[0].init(config.button_pinout.dir_left);
	state::buttons[1].init(config.button_pinout.dir_right);
	state::buttons[2].init(config.button_pinout.dir_up);
	state::buttons[3].init(config.button_pinout.dir_down);
	state::buttons[4].init(config.button_pinout.act_o);
	state::buttons[5].init(config.button_pinout.act_x);

	gpio_set_irq_callback(gpio_irq_handler);
	irq_set_enabled(IO_IRQ_BANK0, true);
}

std::size_t __no_inline_not_in_flash_func(init_psram_pimoroni)() {
	// RP2350 QMI PSRAM initialization code from CircuitPython

	gpio_set_function(config.psram_cs, GPIO_FUNC_XIP_CS1);
	int psram_size;
	psram_size = 0;

	uint32_t interrupt_state = save_and_disable_interrupts();

	// Try and read the PSRAM ID via direct_csr.
	qmi_hw->direct_csr =
		30 << QMI_DIRECT_CSR_CLKDIV_LSB | QMI_DIRECT_CSR_EN_BITS;
	// Need to poll for the cooldown on the last XIP transfer to expire
	// (via direct-mode BUSY flag) before it is safe to perform the first
	// direct-mode operation
	while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0)
		;

	// Exit out of QMI in case we've inited already
	qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;
	// Transmit as quad.
	qmi_hw->direct_tx =
		QMI_DIRECT_TX_OE_BITS |
		QMI_DIRECT_TX_IWIDTH_VALUE_Q << QMI_DIRECT_TX_IWIDTH_LSB | 0xf5;
	while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0)
		;
	(void)qmi_hw->direct_rx;
	qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS);

	// Read the id
	qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;
	uint8_t kgd = 0;
	uint8_t eid = 0;
	for (size_t i = 0; i < 7; i++) {
		if (i == 0) {
			qmi_hw->direct_tx = 0x9f;
		} else {
			qmi_hw->direct_tx = 0xff;
		}
		while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_TXEMPTY_BITS) == 0) {
		}
		while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) {
		}
		if (i == 5) {
			kgd = qmi_hw->direct_rx;
		} else if (i == 6) {
			eid = qmi_hw->direct_rx;
		} else {
			(void)qmi_hw->direct_rx;
		}
	}
	// Disable direct csr.
	qmi_hw->direct_csr &=
		~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS | QMI_DIRECT_CSR_EN_BITS);

	// printf("kgd: %02x\n", kgd);
	/*if (kgd != 0x5D) {
	    common_hal_mcu_enable_interrupts();
	    reset_pin_number(CIRCUITPY_PSRAM_CHIP_SELECT->number);
	    return;
	}
	never_reset_pin_number(CIRCUITPY_PSRAM_CHIP_SELECT->number);*/

	// Enable quad mode.
	qmi_hw->direct_csr =
		30 << QMI_DIRECT_CSR_CLKDIV_LSB | QMI_DIRECT_CSR_EN_BITS;
	// Need to poll for the cooldown on the last XIP transfer to expire
	// (via direct-mode BUSY flag) before it is safe to perform the first
	// direct-mode operation
	while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) {
	}

	// RESETEN, RESET and quad enable
	for (uint8_t i = 0; i < 3; i++) {
		qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;
		if (i == 0) {
			qmi_hw->direct_tx = 0x66;
		} else if (i == 1) {
			qmi_hw->direct_tx = 0x99;
		} else {
			qmi_hw->direct_tx = 0x35;
		}
		while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) {
		}
		qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS);
		for (size_t j = 0; j < 20; j++) {
			asm("nop");
		}
		(void)qmi_hw->direct_rx;
	}
	// Disable direct csr.
	qmi_hw->direct_csr &=
		~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS | QMI_DIRECT_CSR_EN_BITS);

	qmi_hw->m[1].timing =
		QMI_M0_TIMING_PAGEBREAK_VALUE_1024
			<< QMI_M0_TIMING_PAGEBREAK_LSB | // Break between pages.
		3 << QMI_M0_TIMING_SELECT_HOLD_LSB | // Delay releasing CS for 3 extra
	                                         // system cycles.
		3 << QMI_M0_TIMING_COOLDOWN_LSB |
		3 << QMI_M0_TIMING_RXDELAY_LSB |
		32 << QMI_M0_TIMING_MAX_SELECT_LSB | // In units of 64 system clock
	                                         // cycles. PSRAM says 8us max. 8 /
	                                         // 0.00752 / 64 = 16.62
		14 << QMI_M0_TIMING_MIN_DESELECT_LSB | // In units of system clock
	                                           // cycles. PSRAM says 50ns.50
	                                           // / 7.52 = 6.64
		3 << QMI_M0_TIMING_CLKDIV_LSB;
	qmi_hw->m[1].rfmt =
		(QMI_M0_RFMT_PREFIX_WIDTH_VALUE_Q << QMI_M0_RFMT_PREFIX_WIDTH_LSB |
	     QMI_M0_RFMT_ADDR_WIDTH_VALUE_Q << QMI_M0_RFMT_ADDR_WIDTH_LSB |
	     QMI_M0_RFMT_SUFFIX_WIDTH_VALUE_Q << QMI_M0_RFMT_SUFFIX_WIDTH_LSB |
	     QMI_M0_RFMT_DUMMY_WIDTH_VALUE_Q << QMI_M0_RFMT_DUMMY_WIDTH_LSB |
	     QMI_M0_RFMT_DUMMY_LEN_VALUE_24 << QMI_M0_RFMT_DUMMY_LEN_LSB |
	     QMI_M0_RFMT_DATA_WIDTH_VALUE_Q << QMI_M0_RFMT_DATA_WIDTH_LSB |
	     QMI_M0_RFMT_PREFIX_LEN_VALUE_8 << QMI_M0_RFMT_PREFIX_LEN_LSB |
	     QMI_M0_RFMT_SUFFIX_LEN_VALUE_NONE << QMI_M0_RFMT_SUFFIX_LEN_LSB);
	qmi_hw->m[1].rcmd =
		0xeb << QMI_M0_RCMD_PREFIX_LSB | 0 << QMI_M0_RCMD_SUFFIX_LSB;
	qmi_hw->m[1].wfmt =
		(QMI_M0_WFMT_PREFIX_WIDTH_VALUE_Q << QMI_M0_WFMT_PREFIX_WIDTH_LSB |
	     QMI_M0_WFMT_ADDR_WIDTH_VALUE_Q << QMI_M0_WFMT_ADDR_WIDTH_LSB |
	     QMI_M0_WFMT_SUFFIX_WIDTH_VALUE_Q << QMI_M0_WFMT_SUFFIX_WIDTH_LSB |
	     QMI_M0_WFMT_DUMMY_WIDTH_VALUE_Q << QMI_M0_WFMT_DUMMY_WIDTH_LSB |
	     QMI_M0_WFMT_DUMMY_LEN_VALUE_NONE << QMI_M0_WFMT_DUMMY_LEN_LSB |
	     QMI_M0_WFMT_DATA_WIDTH_VALUE_Q << QMI_M0_WFMT_DATA_WIDTH_LSB |
	     QMI_M0_WFMT_PREFIX_LEN_VALUE_8 << QMI_M0_WFMT_PREFIX_LEN_LSB |
	     QMI_M0_WFMT_SUFFIX_LEN_VALUE_NONE << QMI_M0_WFMT_SUFFIX_LEN_LSB);
	qmi_hw->m[1].wcmd =
		0x38 << QMI_M0_WCMD_PREFIX_LSB | 0 << QMI_M0_WCMD_SUFFIX_LSB;

	restore_interrupts(interrupt_state);

	psram_size = 1024 * 1024; // 1 MiB
	uint8_t size_id = eid >> 5;
	if (eid == 0x26 || size_id == 2) {
		psram_size *= 8;
	} else if (size_id == 0) {
		psram_size *= 2;
	} else if (size_id == 1) {
		psram_size *= 4;
	}

	// Mark that we can write to PSRAM.
	xip_ctrl_hw->ctrl |= XIP_CTRL_WRITABLE_M1_BITS;

#ifdef Y8_DEBUG_MEMCHECK
	// half-assed PSRAM test code
	printf("Writing test pattern to PSRAM (through XIP)");
	for (int i = psram_size / 4 - 1; i >= 0; --i) {
		if (i % 8192 == 0) {
			putchar('.');
		}

		volatile uint32_t *test_address =
			reinterpret_cast<uint32_t *>(Y8_EXTMEM_START + i * 4);

		*test_address = unsigned(i);
	}

	printf("\nVerifying test pattern from PSRAM (through XIP)");
	for (int i = 0; i < psram_size / 4; ++i) {
		if (i % 8192 == 0) {
			putchar('.');
		}

		volatile uint32_t *test_address =
			reinterpret_cast<uint32_t *>(Y8_EXTMEM_START + i * 4);
		const auto got = *test_address;
		if (got != unsigned(i)) {
			printf("\nPSRAM self test failed (got %d expected %d at %p)! PSRAM "
			       "heap will be disabled.\n",
			       got, unsigned(i), test_address);
			return 0;
		}
	}
	printf("\nPSRAM self test passed!\n");
#endif

	return psram_size;
}

extern "C" {
extern char __psram_heap_start;
}

void init_video_dwo() {
	spi_init(config.dwo.spi, config.spi_video_clock);
	printf("DO0206FMST01 baudrate: %d\n", spi_get_baudrate(config.dwo.spi));

	asupico::state::dwo.init(config.dwo);

	gpio_set_irq_enabled(config.dwo.pinout.te, GPIO_IRQ_EDGE_RISE, true);
}

} // namespace arch::pico::platform::asupico