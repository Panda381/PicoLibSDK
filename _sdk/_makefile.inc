ifeq (${CPUCLASS},RP2350RISCV)
ASRC += ../../../_sdk/crt0_riscv.S
else
ifeq (${CPUCLASS},RP2350)
ASRC += ../../../_sdk/crt0_rp2350.S
else
ASRC += ../../../_sdk/crt0_rp2040.S
endif
endif

ifeq (${CPUCLASS},RP2040)
ASRC += ../../../_sdk/src/sdk_divider.S
ASRC += ../../../_sdk/src/sdk_memops_asm.S
ASRC += ../../../_sdk/src/sdk_double_m0.S
ASRC += ../../../_sdk/src/sdk_float_m0.S
endif

ifeq (${CPUARCH},M33)
ASRC += ../../../_sdk/src/sdk_double_m33_conv.S
ASRC += ../../../_sdk/src/sdk_double_m33_dcp.S
ASRC += ../../../_sdk/src/sdk_double_m33_fma.S
ASRC += ../../../_sdk/src/sdk_double_m33_sci.S
ASRC += ../../../_sdk/src/sdk_float_m33_conv.S
ASRC += ../../../_sdk/src/sdk_float_m33_dcp.S
ASRC += ../../../_sdk/src/sdk_float_m33_sci_dcp.S
ASRC += ../../../_sdk/src/sdk_float_m33_sci_vfp.S
endif

ifeq (${CPUCLASS},RP2350RISCV)
ASRC += ../../../_sdk/src/sdk_double_riscv.S
ASRC += ../../../_sdk/src/sdk_double_riscv_sci.S
ASRC += ../../../_sdk/src/sdk_float_riscv.S
ASRC += ../../../_sdk/src/sdk_float_riscv_sci.S
endif

CSRC += ../../../_sdk/src/sdk_adc.c
CSRC += ../../../_sdk/src/sdk_bootrom.c
CSRC += ../../../_sdk/src/sdk_clocks.c
CSRC += ../../../_sdk/src/sdk_cpu.c
CSRC += ../../../_sdk/src/sdk_dma.c
CSRC += ../../../_sdk/src/sdk_double.c
CSRC += ../../../_sdk/src/sdk_fatal.c
CSRC += ../../../_sdk/src/sdk_fifo.c
CSRC += ../../../_sdk/src/sdk_flash.c
CSRC += ../../../_sdk/src/sdk_float.c
CSRC += ../../../_sdk/src/sdk_gpio.c
CSRC += ../../../_sdk/src/sdk_hstx.c
CSRC += ../../../_sdk/src/sdk_i2c.c
CSRC += ../../../_sdk/src/sdk_interp.c
CSRC += ../../../_sdk/src/sdk_irq.c
CSRC += ../../../_sdk/src/sdk_multicore.c
CSRC += ../../../_sdk/src/sdk_pio.c
CSRC += ../../../_sdk/src/sdk_pll.c
CSRC += ../../../_sdk/src/sdk_powman.c
CSRC += ../../../_sdk/src/sdk_pwm.c
CSRC += ../../../_sdk/src/sdk_qmi.c
CSRC += ../../../_sdk/src/sdk_qspi.c
CSRC += ../../../_sdk/src/sdk_reset.c
CSRC += ../../../_sdk/src/sdk_rosc.c
CSRC += ../../../_sdk/src/sdk_runtime.c
CSRC += ../../../_sdk/src/sdk_rtc.c
CSRC += ../../../_sdk/src/sdk_spi.c
CSRC += ../../../_sdk/src/sdk_spinlock.c
CSRC += ../../../_sdk/src/sdk_ssi.c
CSRC += ../../../_sdk/src/sdk_systick.c
CSRC += ../../../_sdk/src/sdk_ticks.c
CSRC += ../../../_sdk/src/sdk_timer.c
CSRC += ../../../_sdk/src/sdk_uart.c
CSRC += ../../../_sdk/src/sdk_vreg.c
CSRC += ../../../_sdk/src/sdk_watchdog.c
CSRC += ../../../_sdk/src/sdk_xip.c
CSRC += ../../../_sdk/src/sdk_xosc.c

CSRC += ../../../_sdk/usb_src/sdk_usbport.c

CSRC += ../../../_sdk/usb_src/sdk_usb_dev.c
# CSRC += ../../../_sdk/usb_src/sdk_usb_dev_audio.c
# CSRC += ../../../_sdk/usb_src/sdk_usb_dev_bth.c
CSRC += ../../../_sdk/usb_src/sdk_usb_dev_cdc.c
# CSRC += ../../../_sdk/usb_src/sdk_usb_dev_dfu.c
CSRC += ../../../_sdk/usb_src/sdk_usb_dev_hid.c
# CSRC += ../../../_sdk/usb_src/sdk_usb_dev_midi.c
# CSRC += ../../../_sdk/usb_src/sdk_usb_dev_msc.c
# CSRC += ../../../_sdk/usb_src/sdk_usb_dev_net.c
# CSRC += ../../../_sdk/usb_src/sdk_usb_dev_tmc.c
# CSRC += ../../../_sdk/usb_src/sdk_usb_dev_vendor.c
# CSRC += ../../../_sdk/usb_src/sdk_usb_dev_video.c
CSRC += ../../../_sdk/usb_src/sdk_usb_host.c
CSRC += ../../../_sdk/usb_src/sdk_usb_host_cdc.c
CSRC += ../../../_sdk/usb_src/sdk_usb_host_hid.c
# CSRC += ../../../_sdk/usb_src/sdk_usb_host_msc.c
# CSRC += ../../../_sdk/usb_src/sdk_usb_host_vendor.c
CSRC += ../../../_sdk/usb_src/sdk_usb_phy.c
