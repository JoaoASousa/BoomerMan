PROG=proj

all:
	@echo "Copying project resources..."
	mkdir -p /tmp
	rm -rf /tmp/*
	cp -rp ./resources /tmp
	@echo "Finished copy!"

SRCS = proj.c graphics.c keyboard.c utils.c logic.c mouse.c rtc.c ena_interrupts.S dis_interrupts.S

# __LCOM_OPTIMIZED__ disables most of the LCF's overhead (and functionality!)
CPPFLAGS += -pedantic -D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf

.include <minix.lcom.mk>
