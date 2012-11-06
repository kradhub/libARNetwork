STATIC=yes
DEBUG=no

SOURCES=$(wildcard ../Sources/*.c)
OBJ=$(SOURCES:.c=.o)

TARGET=tbManager

CC=gcc

DIRSDK=/home/m_maitre/Documents/ARDrone/SDK3.0/ARDroneSDK3_0_Install

ifeq ("$(DEBUG)", "no")
CFLAGS=-O3
else
CFLAGS=-O0 -g
endif
CFLAGS+= -I../Includes -I$(DIRSDK)/include -I../../Includes

ifeq ("$(STATIC)", "yes")
LIBS=$(DIRSDK)/lib/libnetWork.a $(DIRSDK)/lib/libsal.a -pthread
else
LDFLAGS= -L$(DIRSDK)/lib -lnetWork -lsal 
#add  path
endif

# Met SILENT=@ pour ne pas qu'il affiche les commandes
SILENT=

.PHONY:all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(SILENT)$(CC) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)

../Sources/%.o: ../Sources/%.c
	$(SILENT)$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(SILENT)rm -f $(TARGET)
	$(SILENT)rm -f $(OBJ)


