$(call add-hdrs,fd_toml.h)
$(call add-objs,fd_toml,fd_util)
$(call make-unit-test,test_toml,test_toml,fd_util)
$(call run-unit-test,test_toml,)