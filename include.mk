GREEN  := \033[1;32m
RED    := \033[1;31m
RESET  := \033[0m

define check_multiboot2
	name=$$(basename $(1) .k) ; \
	msg="multiboot2 compliant" ; \
	grub-file --is-x86-multiboot2 $(1) \
		&&  echo -e "$(GREEN)$$name is $$msg$(RESET)" \
		|| (echo -e "$(RED)$$name is not $$msg$(RESET)" ; exit 1)
endef
