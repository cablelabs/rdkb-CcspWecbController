#######################################################################
#   Copyright [2014] [Cisco Systems, Inc.]
# 
#   Licensed under the Apache License, Version 2.0 (the \"License\");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
# 
#       http://www.apache.org/licenses/LICENSE-2.0
# 
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an \"AS IS\" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#######################################################################

CCSP_SRC_DIR := $(shell pwd)/
MKBASE   = $(CCSP_ROOT_DIR)/$(CCSP_ARCH_DIR_NAME)

include $(MKBASE)/$(CcspArch)/boards/$(CcspBoard)/ccsp_build_common.mk

source_files := $(shell cd $(CCSP_SRC_DIR); find ./ -type f -iname '*.c' )
obj_files := $(subst .c,.o,$(source_files))
obj_files := $(patsubst .%,./build/$(CcspBoard$)%, $(obj_files))
-include $(obj_files:.o=.d)

INCPATH += $(CCSP_OPENSOURCE_DIR)/include/libupnp
INCPATH += $(CCSP_OPENSOURCE_DIR)/include/curl-7.28.1
INCPATH += $(CCSP_OPENSOURCE_DIR)/include/libxml2-2.7.7
INCPATH += $(CCSP_SRC_DIR)
INCFLAGS := $(patsubst %,-I%, $(INCPATH))

all: build/$(CcspBoard$)/wecb_master 

build/$(CcspBoard$)/wecb_master: $(obj_files)
	$(CC) -g -std=c99 $(LDFLAGS) -fPIC $^ -o $@ 

		
build/$(CcspBoard$)/%.o: %.c
	-mkdir -p $(dir $@)
	@$(CC) -g -MD -std=c99 $(CFLAGS) $(INCFLAGS) -fPIC $< -c -o $@  
	
.PHONY: all clean

clean:
	rm -Rf build/$(CcspBoard$)/*

install:
	@-mkdir -p $(CCSP_OUT_DIR)
	cp -f build/$(CcspBoard$)/wecb_master $(CCSP_OUT_DIR)/

LDFLAGS   += -lccsp_common -lpthread 
LDFLAGS	 += -lupnp -lixml
LDFLAGS	 += -lxml2 -lcurl

