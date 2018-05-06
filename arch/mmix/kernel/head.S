/*
 * arch/mmix/kernel/head.S
 *
 *   Copyright (C) 2008,2011 Eiji Yoshiya (eiji-y@pb3.so-net.ne.jp)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <asm/thread_info.h>
#include <asm/asm-offsets.h>

	.text
	.globl	_stext
_stext:
	.globl	__start
__start:
/*
 * TODO:
 * Clear BSS.
 * Copy bootup parameters.
 * ??? Initialize page tables.
 * ??? Enable paging.
 * Setup stack pointer.
 */

/*
 * setup stack for initial 'unsave'.
 *
 * init_thread_union		|-------------------------------|
 *				|				|
 *				|	 thread_info		|
 *				|				|
 *				|-------------------------------|
 *	initial rO, rS -> rL	|		0		| +0 * 8
 *				|-------------------------------|
 *			g[255]	|   ptr to init_thread_union	| +1 * 8
 *				|-------------------------------|
 *				|  rB, rD, rE, rH, rJ, rM	| +2 * 8
 *				|  rR, rP, rW, rX, rY, rZ	|
 *				|-------------------------------|
 *	unsave ->	rG & rA	| 255 | 0	|	rA	| +14 * 8
 *				|-------------------------------|
 */
	set	$0, 0
	geta	$255, init_thread_union
	sto	$0, $255, ((THREAD_INFO_SIZE + 0x7) & ~0x7)	/* rL */
	sto	$255, $255, ((THREAD_INFO_SIZE + 0x7) & ~0x7)+8	/* $255 */
	add	$255, $255, ((THREAD_INFO_SIZE + 0x7) & ~0x7)+(15-1)*8
	seth	$0, 0xff00	/* packed rG = 255 */
	sto	$0, $255, 0	/* | rG | 0 | rA | */
	unsave	$255
	put	rG, 32
	add	$254, $255, 0
	incl	$254, THREAD_SIZE

	geta	$0, forced_trap
	put	rT, $0

	pushj	$0, start_kernel

	trap	0, 0, 0

_savedK:
	octa	savedK
	octa	intrEn
_aux_reg:
	octa	aux_reg

	.globl	dynamic_trap
dynamic_trap:
	geta	$255,_aux_reg
	ldo	$255,$255,0
	ldo	$255,$255,AUX_REG_UCP
	save	$255,0	#
	/* user mode ? */
	get	$0,rWW
	bn	$0,1f
	geta	$254,_aux_reg
	ldo	$254,$254,0
	sto	$255,$254,AUX_REG_USP
	ldo	$255,$254,AUX_REG_KSP
	unsave	0,$255
	pushj	$0,do_IRQ
	jmp	ret_to_user

/* trapped in kernel mode */
1:
	set	$0,$255 #
	pushj	$1,do_IRQ
	set	$255,$0	#

	unsave	0,$255	#
	geta	$255,_savedK
	ldo	$255,$255,0
	ldo	$255,$255,0
	resume	1

	.globl	forced_trap
forced_trap:
	get	$255,rXX
	bn	$255,syscall
	get	$255,rJ
	pushj	$255,emulate_mmix
	put	rJ,$255
	geta	$255,_savedK
	ldo	$255,$255,8	// intrEn
	ldo	$255,$255,0
	bz	$255,1f
	geta	$255,_savedK
	ldo	$255,$255,0
	ldo	$255,$255,0
1:
	resume	1
syscall:
	geta	$255,_aux_reg
	ldo	$255,$255,0
	ldo	$255,$255,AUX_REG_UCP
	save	$255,0		/* save user stack */
	geta	$254,_aux_reg
	ldo	$254,$254,0
	sto	$255,$254,AUX_REG_USP
	ldo	$255,$254,AUX_REG_KSP
	unsave	0,$255		/* load kernel stack */
	set	$0,$255
	pushj	$0,do_syscall
ret_to_user:
	save	$255,0
	geta	$0,_aux_reg
	ldo	$0,$0,0
	sto	$255,$0,AUX_REG_KSP
	ldo	$255,$0,AUX_REG_UCP
	unsave	0,$255
	geta	$255,_savedK
	ldo	$255,$255,0
	ldo	$255,$255,0
	ormh	$255,0x0010	/* k-bit on */
	resume	1

	.globl	ret_from_syscall
ret_from_syscall:
	pushj	$0,schedule_tail
	set	$255,0
	put	rK,$255
	get	$255,rB
	put	rBB,$255
	get	$255,rW
	put	rWW,$255
	get	$255,rX
	put	rXX,$255
	get	$255,rY
	put	rYY,$255
	get	$255,rZ
	put	rZZ,$255
	save	$255,0
	geta	$0,_aux_reg
	ldo	$0,$0,0
	sto	$255,$0,AUX_REG_KSP
	ldo	$255,$0,AUX_REG_UCP
	unsave	0,$255
	geta	$255,_savedK
	ldo	$255,$255,0
	ldo	$255,$255,0
	ormh	$255,0x0010	/* k-bit on */
	resume	1
	
	.globl	syscall_ret
syscall_ret:
	set	$255,$0
	unsave	0,$255
	geta	$255,_savedK
	ldo	$255,$255,0
	ldo	$255,$255,0
	ormh	$255,0x0010	/* k-bit on */
	resume	1

	.globl	_switch
_switch:
	set	$251,$0		/* from */
	set	$252,$1		/* to */
	save	$255,0
	stou	$255,$251,KSP
	ldou	$255,$252,KSP
	subu	$252,$255,104	/* 104 = offset of r255 */
	stou	$251,$252
	unsave	0,$255
	set	$0,THREAD
	sub	$0,$255,$0
	pop	1,0

	.data
	.globl	aux_reg
aux_reg:
	.space	AUX_REG_SIZE

	.globl	empty_zero_page
empty_zero_page:
	.space	8192

	.globl	swapper_pg_dir
swapper_pg_dir:
	.space	8192

.section .rodata,"a"
	.align	8
	.globl	sys_call_table
sys_call_table:
	octa	sys_restart_syscall	/* 0 */
	octa	sys_exit
	octa	sys_fork
	octa	sys_read
	octa	sys_write
	octa	sys_open		/* 5 */
	octa	sys_close
	octa	sys_waitpid
	octa	sys_creat
	octa	sys_link
	octa	sys_unlink	/* 10 */
	octa	sys_execve
	octa	sys_chdir
	octa	sys_time
	octa	sys_mknod
	octa	sys_chmod		/* 15 */
	octa	sys_lchown
	octa	sys_ni_syscall	/* old break syscall holder */
	octa	sys_ni_syscall	/* old stat syscall holder */
	octa	sys_lseek
	octa	sys_getpid	/* 20 */
	octa	sys_mount
	octa	sys_oldumount
	octa	sys_setuid
	octa	sys_getuid
	octa	sys_stime		/* 25 */
	octa	sys_ptrace
	octa	sys_alarm
	octa	sys_ni_syscall	/* old fstat syscall holder */
	octa	sys_pause
	octa	sys_utime		/* 30 */
	octa	sys_ni_syscall	/* old stty syscall holder */
	octa	sys_ni_syscall	/* old gtty syscall holder */
	octa	sys_access
	octa	sys_nice
	octa	sys_ni_syscall	/* 35 - old ftime syscall holder */
	octa	sys_sync
	octa	sys_kill
	octa	sys_rename
	octa	sys_mkdir
	octa	sys_rmdir		/* 40 */
	octa	sys_dup
	octa	sys_pipe
	octa	sys_times
	octa	sys_ni_syscall	/* old prof syscall holder */
	octa	sys_brk		/* 45 */
	octa	sys_setgid
	octa	sys_getgid
	octa	sys_signal
	octa	sys_geteuid
	octa	sys_getegid	/* 50 */
	octa	sys_acct
	octa	sys_umount
	octa	sys_ni_syscall	/* old lock syscall holder */
	octa	sys_ioctl
	octa	sys_fcntl		/* 55 */
	octa	sys_ni_syscall	/* old mpx syscall holder */
	octa	sys_setpgid
	octa	sys_ni_syscall	/* old ulimit syscall holder */
	octa	sys_olduname
	octa	sys_umask		/* 60 */
	octa	sys_chroot
	octa	sys_ustat
	octa	sys_dup2
	octa	sys_getppid
	octa	sys_getpgrp	/* 65 */
	octa	sys_setsid
	octa	sys_sigaction
	octa	sys_sgetmask
	octa	sys_ssetmask
	octa	sys_setreuid	/* 70 */
	octa	sys_setregid
	octa	sys_sigsuspend
	octa	sys_sigpending
	octa	sys_sethostname
	octa	sys_setrlimit	/* 75 */
	octa	sys_old_getrlimit
	octa	sys_getrusage
	octa	sys_gettimeofday
	octa	sys_settimeofday
	octa	sys_getgroups	/* 80 */
	octa	sys_setgroups
	octa	old_select
	octa	sys_symlink
	octa	sys_ni_syscall	/* old lstat syscall holder */
	octa	sys_readlink	/* 85 */
	octa	sys_uselib
	octa	sys_swapon
	octa	sys_reboot
	octa	old_readdir
	octa	sys_mmap		/* 90 */
	octa	sys_munmap
	octa	sys_truncate
	octa	sys_ftruncate
	octa	sys_fchmod
	octa	sys_fchown	/* 95 */
	octa	sys_getpriority
	octa	sys_setpriority
	octa	sys_ni_syscall	/* old profil syscall holder */
	octa	sys_statfs
	octa	sys_fstatfs	/* 100 */
	octa	sys_ni_syscall	/* i386 ioperm syscall holder */
	octa	sys_socketcall
	octa	sys_syslog
	octa	sys_setitimer
	octa	sys_getitimer	/* 105 */
	octa	sys_newstat
	octa	sys_newlstat
	octa	sys_newfstat
	octa	sys_uname
	octa	sys_ni_syscall	/* 110 i386 iopl syscall holder */
	octa	sys_vhangup
	octa	sys_ni_syscall	/* old "idle" system call */
	octa	sys_ni_syscall	/* i386 vm86old syscall hodler */
	octa	sys_wait4
	octa	sys_swapoff	/* 115 */
	octa	sys_sysinfo
	octa	sys_ipc
	octa	sys_fsync
	octa	sys_sigreturn
	octa	sys_clone		/* 120 */
	octa	sys_setdomainname
	octa	sys_newuname
	octa	sys_ni_syscall		/* i386 modify_ldt syscall holder */
	octa	sys_adjtimex
	octa	sys_mprotect	/* 125 */
	octa	sys_sigprocmask
	octa	sys_ni_syscall	/* old "create_module" */
	octa	sys_init_module
	octa	sys_delete_module
	octa	sys_ni_syscall	/* 130:	old "get_kernel_syms" */
	octa	sys_quotactl
	octa	sys_getpgid
	octa	sys_fchdir
	octa	sys_bdflush
	octa	sys_sysfs		/* 135 */
	octa	sys_personality
	octa	sys_ni_syscall	/* reserved for afs_syscall */
	octa	sys_setfsuid
	octa	sys_setfsgid
	octa	sys_llseek	/* 140 */
	octa	sys_getdents
	octa	sys_select
	octa	sys_flock
	octa	sys_msync
	octa	sys_readv		/* 145 */
	octa	sys_writev
	octa	sys_getsid
	octa	sys_fdatasync
	octa	sys_sysctl
	octa	sys_mlock		/* 150 */
	octa	sys_munlock
	octa	sys_mlockall
	octa	sys_munlockall
	octa	sys_sched_setparam
	octa	sys_sched_getparam   /* 155 */
	octa	sys_sched_setscheduler
	octa	sys_sched_getscheduler
	octa	sys_sched_yield
	octa	sys_sched_get_priority_max
	octa	sys_sched_get_priority_min  /* 160 */
	octa	sys_sched_rr_get_interval
	octa	sys_nanosleep
	octa	sys_mremap
	octa	sys_setresuid
	octa	sys_getresuid	/* 165 */
	octa	sys_ni_syscall	/* Old sys_query_module */
	octa	sys_poll
	octa	sys_nfsservctl
	octa	sys_setresgid
	octa	sys_getresgid	/* 170 */
	octa	sys_prctl
	octa	sys_rt_sigreturn
	octa	sys_rt_sigaction
	octa	sys_rt_sigprocmask
	octa	sys_rt_sigpending	/* 175 */
	octa	sys_rt_sigtimedwait
	octa	sys_rt_sigqueueinfo
	octa	sys_rt_sigsuspend
	octa	sys_pread64
	octa	sys_pwrite64	/* 180 */
	octa	sys_chown
	octa	sys_getcwd
	octa	sys_capget
	octa	sys_capset
	octa	sys_sigaltstack	/* 185 */
	octa	sys_sendfile
	octa	sys_ni_syscall	/* reserved for streams1 */
	octa	sys_ni_syscall	/* reserved for streams2 */
	octa	sys_vfork
	octa	sys_getrlimit		/* 190 */
	octa	sys_readahead
	octa	sys_ni_syscall	/* sys_mmap2 */
	octa	sys_ni_syscall	/* sys_truncate64 */
	octa	sys_ni_syscall	/* sys_ftruncate64 */
	octa	sys_stat64	/* 195 */
	octa	sys_lstat64
	octa	sys_fstat64
	octa	sys_ni_syscall	/* sys_pciconfig_read */
	octa	sys_ni_syscall	/* sys_pciconfig_write */
	octa	sys_ni_syscall	/* 200 sys_pciconfig_iobase */
	octa	sys_ni_syscall	/* sys_multiplexer */
	octa	sys_getdents64
	octa	sys_pivot_root
	octa	sys_ni_syscall	/* sys_fcntl64 */
	octa	sys_madvise	/* 205 */
	octa	sys_mincore
	octa	sys_gettid
	octa	sys_tkill
	octa	sys_setxattr
	octa	sys_lsetxattr	/* 210 */
	octa	sys_fsetxattr
	octa	sys_getxattr
	octa	sys_lgetxattr
	octa	sys_fgetxattr
	octa	sys_listxattr	/* 215 */
	octa	sys_llistxattr
	octa	sys_flistxattr
	octa	sys_removexattr
	octa	sys_lremovexattr
	octa	sys_removexattr	/* 220 */
	octa	sys_futex
	octa	sys_sched_setaffinity
	octa	sys_sched_getaffinity
	octa	sys_ni_syscall
	octa	sys_ni_syscall	/* 225 */
	octa	sys_sendfile64
	octa	sys_io_setup
	octa	sys_io_destroy
	octa	sys_io_getevents
	octa	sys_io_submit	/* 230 */
	octa	sys_io_cancel
	octa	sys_set_tid_address
	octa	sys_fadvise64
	octa	sys_exit_group
	octa	sys_lookup_dcookie	/* 235 */
	octa	sys_epoll_create
	octa	sys_epoll_ctl
	octa	sys_epoll_wait
	octa	sys_remap_file_pages
	octa	sys_timer_create	/* 240 */
	octa	sys_timer_settime
	octa	sys_timer_gettime
	octa	sys_timer_getoverrun
	octa	sys_timer_delete
	octa	sys_clock_settime	/* 245 */
	octa	sys_clock_gettime
	octa	sys_clock_getres
	octa	sys_clock_nanosleep
	octa	sys_ni_syscall		/* sys_swapcontext */
	octa	sys_tgkill		/* 250 */
	octa	sys_utimes
	octa	sys_statfs64
	octa	sys_fstatfs64
	octa	sys_fadvise64_64
	octa	sys_ni_syscall		/* 255 sys_rtas	*/
	octa	sys_ni_syscall		/* sys_debug_setcontext */
	octa	sys_ni_syscall
	octa	sys_ni_syscall
	octa	sys_mbind
	octa	sys_get_mempolicy	/* 260 */
	octa	sys_set_mempolicy
	octa	sys_mq_open
	octa	sys_mq_unlink
	octa	sys_mq_timedsend
	octa	sys_mq_timedreceive	/* 265 */
	octa	sys_mq_notify
	octa	sys_mq_getsetattr
	octa	sys_kexec_load
	octa	sys_add_key
	octa	sys_request_key		/* 270 */
	octa	sys_keyctl
	octa	sys_waitid
	octa	sys_ioprio_set
	octa	sys_ioprio_get
	octa	sys_inotify_init	/* 275 */
	octa	sys_inotify_add_watch
	octa	sys_inotify_rm_watch
	octa	sys_spu_run
	octa	sys_spu_create
	octa	sys_pselect6		/* 280 */
	octa	sys_ppoll
	octa	sys_unshare
	octa	sys_splice
	octa	sys_tee
	octa	sys_vmsplice		/* 285 */
	octa	sys_openat
	octa	sys_mkdirat
	octa	sys_mknodat
	octa	sys_fchownat
	octa	sys_futimesat		/* 290 */
	octa	sys_newfstatat
	octa	sys_unlinkat
	octa	sys_renameat
	octa	sys_linkat
	octa	sys_symlinkat		/* 295 */
	octa	sys_readlinkat
	octa	sys_fchmodat
	octa	sys_faccessat
	octa	sys_get_robust_list
	octa	sys_set_robust_list	/* 300 */

	.globl	nr_syscalls
nr_syscalls:
	octa	(. - sys_call_table)/8
