/*
 * arch/mmix/kernel/syscalls.c
 *
 *   Copyright (C) 2008 Eiji Yoshiya (eiji-y@pb3.so-net.ne.jp)
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

#include <linux/errno.h>
#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/utsname.h>
#include <linux/syscalls.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/file.h>
#include <linux/sem.h>
#include <linux/msg.h>
#include <linux/shm.h>
#include <linux/ipc.h>

#include <asm/uaccess.h>
#include <asm/ipc.h>

int sys_ipc(uint call, int first, unsigned long second, long third,
	    void __user *ptr, long fifth)
{
	int version, ret;

	version = call >> 16; /* hack for backward compatibility */
	call &= 0xffff;

	ret = -ENOSYS;
	switch (call) {
	case SEMOP:
		ret = sys_semtimedop(first, (struct sembuf __user *)ptr,
				      (unsigned)second, NULL);
		break;
	case SEMTIMEDOP:
		ret = sys_semtimedop(first, (struct sembuf __user *)ptr,
				      (unsigned)second,
				      (const struct timespec __user *) fifth);
		break;
	case SEMGET:
		ret = sys_semget (first, (int)second, third);
		break;
	case SEMCTL: {
		union semun fourth;

		ret = -EINVAL;
		if (!ptr)
			break;
		if ((ret = get_user(fourth.__pad, (void __user * __user *)ptr)))
			break;
		ret = sys_semctl(first, (int)second, third, fourth);
		break;
	}
	case MSGSND:
		ret = sys_msgsnd(first, (struct msgbuf __user *)ptr,
				 (size_t)second, third);
		break;
	case MSGRCV:
		switch (version) {
		case 0: {
			struct ipc_kludge tmp;

			ret = -EINVAL;
			if (!ptr)
				break;
			if ((ret = copy_from_user(&tmp,
						(struct ipc_kludge __user *) ptr,
						sizeof (tmp)) ? -EFAULT : 0))
				break;
			ret = sys_msgrcv(first, tmp.msgp, (size_t) second,
					  tmp.msgtyp, third);
			break;
		}
		default:
			ret = sys_msgrcv (first, (struct msgbuf __user *) ptr,
					  (size_t)second, fifth, third);
			break;
		}
		break;
	case MSGGET:
		ret = sys_msgget((key_t)first, (int)second);
		break;
	case MSGCTL:
		ret = sys_msgctl(first, (int)second,
				  (struct msqid_ds __user *)ptr);
		break;
	case SHMAT: {
		ulong raddr;
		ret = do_shmat(first, (char __user *)ptr, (int)second, &raddr);
		if (ret)
			break;
		ret = put_user(raddr, (ulong __user *) third);
		break;
	}
	case SHMDT:
		ret = sys_shmdt((char __user *)ptr);
		break;
	case SHMGET:
		ret = sys_shmget(first, (size_t)second, third);
		break;
	case SHMCTL:
		ret = sys_shmctl(first, (int)second,
				 (struct shmid_ds __user *)ptr);
		break;
	}

	return ret;
}

int sys_pipe(int __user *fildes)
{
	int fd[2];
	int error;

	error = do_pipe(fd);
	if (!error) {
		if (copy_to_user(fildes, fd, 2*sizeof(int)))
			error = -EFAULT;
	}
	return error;
}

static inline unsigned long do_mmap2(unsigned long addr, size_t len,
			unsigned long prot, unsigned long flags,
			unsigned long fd, unsigned long off, int shift)
{
	struct file * file = NULL;
	unsigned long ret = -EINVAL;

	if (shift) {
		if (off & ((1 << shift) - 1))
			goto out;
		off >>= shift;
	}
		
	ret = -EBADF;
	if (!(flags & MAP_ANONYMOUS)) {
		if (!(file = fget(fd)))
			goto out;
	}

	flags &= ~(MAP_EXECUTABLE | MAP_DENYWRITE);

	down_write(&current->mm->mmap_sem);
	ret = do_mmap_pgoff(file, addr, len, prot, flags, off);
	up_write(&current->mm->mmap_sem);
	if (file)
		fput(file);
out:
	return ret;
}

unsigned long sys_mmap2(unsigned long addr, size_t len,
			unsigned long prot, unsigned long flags,
			unsigned long fd, unsigned long pgoff)
{
	return do_mmap2(addr, len, prot, flags, fd, pgoff, PAGE_SHIFT-12);
}

unsigned long sys_mmap(unsigned long addr, size_t len,
		       unsigned long prot, unsigned long flags,
		       unsigned long fd, off_t offset)
{
	return do_mmap2(addr, len, prot, flags, fd, offset, PAGE_SHIFT);
}

int sys_uname(struct old_utsname __user * name)
{
	int err;
	if (!name)
		return -EFAULT;
	down_read(&uts_sem);
	err=copy_to_user(name, &system_utsname, sizeof (*name));
	up_read(&uts_sem);
	return err?-EFAULT:0;
}

int sys_olduname(struct oldold_utsname __user *name)
{
	int error;

	if (!name)
		return -EFAULT;
	if (!access_ok(VERIFY_WRITE,name,sizeof(struct oldold_utsname)))
		return -EFAULT;
  
  	down_read(&uts_sem);
	
	error = __copy_to_user(&name->sysname,&system_utsname.sysname,__OLD_UTS_LEN);
	error |= __put_user(0,name->sysname+__OLD_UTS_LEN);
	error |= __copy_to_user(&name->nodename,&system_utsname.nodename,__OLD_UTS_LEN);
	error |= __put_user(0,name->nodename+__OLD_UTS_LEN);
	error |= __copy_to_user(&name->release,&system_utsname.release,__OLD_UTS_LEN);
	error |= __put_user(0,name->release+__OLD_UTS_LEN);
	error |= __copy_to_user(&name->version,&system_utsname.version,__OLD_UTS_LEN);
	error |= __put_user(0,name->version+__OLD_UTS_LEN);
	error |= __copy_to_user(&name->machine,&system_utsname.machine,__OLD_UTS_LEN);
	error |= __put_user(0,name->machine+__OLD_UTS_LEN);
	
	up_read(&uts_sem);
	
	error = error ? -EFAULT : 0;

	return error;
}

struct sel_arg_struct {
	unsigned long n;
	fd_set __user *inp, *outp, *exp;
	struct timeval __user *tvp;
};

int old_select(struct sel_arg_struct __user *arg)
{
	struct sel_arg_struct a;

	if (copy_from_user(&a, arg, sizeof(a)))
		return -EFAULT;
	/* sys_select() does the appropriate kernel locking */
	return sys_select(a.n, a.inp, a.outp, a.exp, a.tvp);
}
