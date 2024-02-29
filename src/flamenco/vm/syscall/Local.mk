ifdef FD_HAS_INT128
$(call add-hdrs,fd_vm_syscall.h)
$(call add-objs,fd_vm_syscall_util fd_vm_syscall_pda fd_vm_syscall_crypto fd_vm_syscall_curve,fd_flamenco)
$(call make-unit-test,test_vm_syscall_curve,test_vm_syscall_curve,fd_flamenco fd_funk fd_ballet fd_util)
$(call run-unit-test,test_vm_syscall_curve)
endif
