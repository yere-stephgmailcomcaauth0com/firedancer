$(call add-hdrs,fd_sha256.h)
$(call add-objs,fd_sha256,fd_ballet)
$(call add-asms,fd_sha256_core_shaext,fd_ballet)
$(call add-objs,fd_sha256_batch_avx,fd_ballet)

$(call make-unit-test,test_sha256,test_sha256,fd_ballet fd_util)
$(call run-unit-test,test_sha256)
