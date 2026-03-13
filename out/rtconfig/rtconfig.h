#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* Fullhan RTOS Configuration */

/* Kernel Feature */

/* RT-Thread Kernel */

#define RT_NAME_MAX 16
/* RT_USING_SMP is not set */
#define RT_ALIGN_SIZE 4
/* RT_THREAD_PRIORITY_8 is not set */
/* RT_THREAD_PRIORITY_32 is not set */
#define RT_THREAD_PRIORITY_256
#define RT_THREAD_PRIORITY_MAX 256
#define RT_TICK_PER_SECOND 100
#define MAX_HANDLERS 256
#define RT_USING_VFP
#define RT_USING_HOOK
#define RT_USING_IDLE_HOOK
#define RT_IDEL_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 1024
/* RT_USING_CPU_IDLE is not set */
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO 8
#define RT_TIMER_THREAD_STACK_SIZE 2048
#define USER_INIT_THREAD_STACK_SIZE 32768
#define RT_DEBUG
#define RT_DEBUG_COLOR
/* RT_RUNLOG is not set */
/* RT_USING_BACKTRACE is not set */
/* RT_USING_MM_TRACE is not set */
#define RT_USING_OVERFLOW_CHECK
/* RT_DEBUG_INIT_CONFIG is not set */
/* RT_DEBUG_THREAD_CONFIG is not set */
/* RT_DEBUG_SCHEDULER_CONFIG is not set */
/* RT_DEBUG_IPC_CONFIG is not set */
/* RT_DEBUG_TIMER_CONFIG is not set */
/* RT_DEBUG_IRQ_CONFIG is not set */
/* RT_DEBUG_MEM_CONFIG is not set */
/* RT_DEBUG_SLAB_CONFIG is not set */
/* RT_DEBUG_MEMHEAP_CONFIG is not set */
/* RT_DEBUG_MODULE_CONFIG is not set */
/* RT_DETECT_INFINITE_LOOP is not set */

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE
/* RT_USING_SIGNALS is not set */
#define RT_USING_THREAD_LOCK

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
/* RT_USING_NOHEAP is not set */
/* RT_USING_SMALL_MEM is not set */
#define RT_USING_SLAB
/* RT_USING_MEMHEAP_AS_HEAP is not set */
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
/* RT_USING_DEVICE_OPS is not set */
#define RT_USING_INTERRUPT_INFO
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 128
#define RT_CONSOLE_DEVICE_NAME "uart0"
#define RT_VER_NUM 0x30103

/* C standard library */

#define RT_USING_LIBC
#define RT_USING_NEWLIB
#define RT_USING_POSIX
/* RT_USING_POSIX_MMAP is not set */
/* RT_USING_POSIX_TERMIOS is not set */
/* RT_USING_POSIX_AIO is not set */
#define RT_USING_TIMEKEEPING

/* C++ features */

/* RT_USING_CPLUSPLUS is not set */

/* PM features */

/* RT_USING_PM is not set */

/* Platform Configuration */

#define RT_USING_CALIBRATE
#define CONFIG_ARCH_FULLHAN
/* CONFIG_ARCH_FH8862 is not set */
/* CONFIG_ARCH_MC632X is not set */
/* CONFIG_ARCH_FH885xV500 is not set */
#define CONFIG_ARCH_FH8626V300
#define FH_USING_AON_PIN
/* CONFIG_CHIP_FH8862 is not set */
/* CONFIG_CHIP_MC632X is not set */
/* CONFIG_CHIP_FH8856V500 is not set */
/* CONFIG_CHIP_FH8857V500 is not set */
#define CONFIG_CHIP_FH8626V300
#define FH_USING_VMM
/* FH_USING_PSRAM is not set */
/* FH_FAST_BOOT is not set */
#define CONFIG_BOARD_APP
/* CONFIG_BOARD_TEST is not set */
#define FH_ENABLE_SELF_LOAD_CODE
#define FH_CONFIG_COMPRESS_LZO
/* FH_CONFIG_COMPRESS_GZIP is not set */
#define ARCH_ARM
#define ARCH_ARM_ARM11

/* Drivers */

/* Usb configuration */

/* choose only one(usb host/device mode) */

#define RT_USING_USB
/* RT_USING_UHC is not set */
#define RT_USING_UDC
/* RT_USING_USB_DEVICE is not set */
#define FH_RT_USB_DEVICE
#define FH_RT_USB_DEVICE_COMPOSITE
#define FH_RT_USB_DEVICE_UVC
/* FH_RT_USB_DEVICE_UAC is not set */
/* FH_RT_USB_DEVICE_CDC is not set */
/* FH_RT_USB_DEVICE_HID is not set */
#define FH_RT_USB_DEVICE_NONE
/* FH_RT_USB_DEVICE_MSTORAGE is not set */
/* FH_RT_USB_DEVICE_RNDIS is not set */
#define DEVICE_VENDOR_LABEL "webcamvendor"
#define DEVICE_PRODUCT_LABEL "webcamproduct"
#define DEVICE_SERIALNUM_LABEL "00000000"
#define UVC_STRING_ASSOCIATION "usb-webcam"
/* UVC_DOUBLE_STREAM is not set */
/* UVC_STILL_IMAGE_CAPTURE_METHOD2 is not set */
/* FH_RT_UVC_EP_TYPE_BULK is not set */
#define FH_RT_UVC_EP_TYPE_ISOC
#define UVC_PACKET_SIZE 1024
/* ISOC_HIGH_BANDWIDTH_EP is not set */

/* WiFi Configuration */

/* FH_USING_WIFI is not set */

/* Choose uart device */

#define FH_USING_UART0
/* FH_USING_UART1 is not set */
/* FH_USING_UART2 is not set */
/* FH_USING_UART_DMA is not set */
/* FH_USING_AES is not set */
#define FH_USING_CESA
#define FH_USING_EFUSE
#define FH_EFUSE_V1
/* FH_EFUSE_V2 is not set */
/* FH_USING_ACW is not set */
#define FH_USING_CLOCK
#define FH_USING_AON_CLK
#define FH_USING_MOL_DMA
#define FH_USING_DMA_MEM
#define FH_USING_FLASH
#define RT_MAX_FAL_PART_NUM 10
#define RT_USING_SFUD
/* RT_SFUD_USING_QSPI is not set */
/* CTL_DUAL_WIRE_SUPPORT is not set */
/* CTL_QUAD_WIRE_SUPPORT is not set */
#define RT_USING_FAL_SFUD_ADAPT
/* FH_USING_NAND_FLASH is not set */
#define SFUD_USING_SFDP
#define SFUD_USING_FLASH_INFO_TABLE
#define RT_CAPACITYID_DISCRIMINATE_FLASH
#define FH_USING_GMAC
#define RT_USING_GPIO
#define RT_USING_DW_GPIO
/* RT_USING_MOL_GPIO is not set */
/* RT_USING_I2C is not set */
/* FH_USING_I2S is not set */
#define RT_USING_SDIO
#define RT_MMCSD_MAX_PARTITION 16
#define RT_USING_MTD
#define RT_USING_MTD_NOR
/* RT_USING_MTD_NAND is not set */
#define RT_USING_PWM
/* RT_USING_RTC is not set */
/* FH_USING_SADC is not set */
#define RT_USING_SERIAL
#define RT_USING_SPI
/* RT_USING_SPISLAVE is not set */
#define RT_USING_TIMER
#define RT_USING_DW_TIMER
/* RT_USING_MOL_TIMER is not set */
#define RT_USING_WDT
#define FH_USING_FH_PERF
#define FH_PERF_V2
#define FH_USING_FH_STEPMOTOR
#define FH_STEPMOTOR_V1_1
#define FH_STEPMOTOR_V1_2
/* FH_USING_FH_HASH is not set */
#define FH_USING_HWSPINLOCK
#define FH_USING_MODULE_CASE
#define FH_USING_MODULE_CASE_RPC
/* FH_USING_MODULE_CASE_LOCAL is not set */

/* Compatibility Interface */

#define RT_USING_COMPONENTS_LINUX_ADAPTER
#define __LINUX_ERRNO_EXTENSIONS__
#define RT_USING_PTHREADS

/* Device virtual file system */

#define RT_USING_DFS
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 4
#define DFS_FILESYSTEM_TYPES_MAX 4
#define DFS_FD_MAX 256
/* RT_USING_DFS_MNTTABLE is not set */
#define RT_USING_DFS_ELMFAT
#define RT_DFS_ELM_BLK_CACHE

/* elm-chan's FatFs, Generic FAT Filesystem Module */

#define RT_DFS_ELM_CODE_PAGE_FILE
#define RT_DFS_ELM_WORD_ACCESS
/* RT_DFS_ELM_USE_LFN_0 is not set */
/* RT_DFS_ELM_USE_LFN_1 is not set */
#define RT_DFS_ELM_USE_LFN_2
/* RT_DFS_ELM_USE_LFN_3 is not set */
#define RT_DFS_ELM_USE_LFN 2
#define RT_DFS_ELM_MAX_LFN 128
#define RT_DFS_ELM_DRIVES 2
#define RT_DFS_ELM_MAX_SECTOR_SIZE 512
/* RT_DFS_ELM_USE_ERASE is not set */
#define RT_DFS_ELM_REENTRANT
#define RT_DFS_ELM_USE_EXFAT
#define RT_USING_DFS_DEVFS
/* RT_USING_DFS_ROMFS is not set */
/* RT_USING_DFS_RAMFS is not set */
/* RT_USING_DFS_UFFS is not set */
#define RT_USING_DFS_JFFS2
/* RT_USING_DFS_YAFFS2 is not set */
/* RT_USING_DFS_NFS is not set */
/* RT_USING_MTD_UBI is not set */

/* light weight TCP/IP stack */

#define RT_USING_LWIP
/* RT_LWIP_SNMP is not set */
#define RT_LWIP_DNS
/* RT_USING_TFTP is not set */
/* RT_USING_TELNET is not set */
#define RT_LWIP_DHCP
#define IP_SOF_BROADCAST 1
#define IP_SOF_BROADCAST_RECV 1

/* Static IPv4 Address */

#define RT_LWIP_IPADDR "192.168.1.30"
#define RT_LWIP_GWADDR "192.168.1.1"
#define RT_LWIP_MSKADDR "255.255.255.0"
/* RT_USING_DHCPD is not set */
/* RT_LWIP_NETIF_LOOPBACK is not set */
#define LWIP_NETIF_LOOPBACK 0
#define RT_LWIP_FULLDUPLEX
/* RT_LWIP_PPP is not set */
#define RT_USING_LWIP_IPV6
#define RT_LWIP_IGMP
#define RT_LWIP_ICMP
#define RT_LWIP_UDP
#define RT_LWIP_TCP
#define RT_LWIP_RAW
/* RT_LWIP_RAWLINK is not set */
#define RT_LWIP_CHECKSUM_GEN
#define RT_LWIP_CHECKSUM_CHECK
/* RT_LWIP_TCP_MSS is not set */
#define RT_LWIP_REASSEMBLY_FRAG
#define RT_LWIP_NETIF_STATUS_CALLBACK
#define RT_LWIP_SO_REUSE
#define RT_LWIP_SO_RCVTIMEO
#define RT_LWIP_SO_SNDTIMEO
#define RT_LWIP_SO_RCVBUF
/* RT_LWIP_SO_LINGER is not set */
/* RT_LWIP_STATS is not set */
#define RT_LWIP_TCPTHREAD_PRIORITY 100
#define RT_LWIP_TCPTHREAD_MBOX_SIZE 32
#define RT_LWIP_TCPTHREAD_STACKSIZE 8192
/* LWIP_NO_RX_THREAD is not set */
/* LWIP_NO_TX_THREAD is not set */
#define RT_LWIP_ETHTHREAD_PRIORITY 126
#define RT_LWIP_ETHTHREAD_STACKSIZE 1024
#define RT_LWIP_ETHTHREAD_MBOX_SIZE 32
/* RT_LWIP_DEBUG is not set */

/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 20
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
/* FINSH_ECHO_DISABLE_DEFAULT is not set */
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 15360
#define FINSH_CMD_SIZE 256
/* FINSH_USING_AUTH is not set */
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_USING_MSH_ONLY
#define FINSH_ARG_MAX 32

/* components packages */

#define PKG_USING_OPTPARSE
#define FH_USING_XBUS
#define FH_USING_PROFILING
/* PKG_USING_VI is not set */
/* RT_USING_WEBCLIENT is not set */
/* FH_USING_YMODEM is not set */
/* RT_USING_MOBILE_TELE is not set */

/* ulog */

/* RT_USING_ULOG is not set */

/* external components */

/* FH_USING_BONJOUR is not set */
/* FH_USING_MBEDTLS is not set */
/* FH_USING_LIBCURL is not set */
/* FH_USING_UDT is not set */
/* FH_USING_SRT is not set */
/* FH_USING_MINIUPNPC is not set */
/* FH_USING_LZMA is not set */
#define FH_USING_LZOP
/* FH_USING_IPERF is not set */
#define FH_USING_ZLIB
/* FH_USING_CJSON is not set */
#define FH_USING_FHCRC
/* FH_USING_MXML is not set */
/* FH_USING_ZBAR is not set */
/* FH_USING_TAR is not set */
/* CONFIG_TEST_LIB is not set */
/* APP_CONFIG_HELLOWORLD is not set */
#define APP_CONFIG_MEDIA_DEMO
/* APP_CONFIG_BSP_DEMO is not set */
/* APP_CONFIG_AUDIO_DEMO is not set */
/* APP_CONFIG_RPC_VIDEO is not set */

#endif
