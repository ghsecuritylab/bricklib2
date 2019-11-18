DOCKER_LOCK_FILE                := $(ROOT_DIR)/DOCKER_LOCK
STM32CUBEF0_PATH                := $(realpath $(BRICKLIB2_PATH)/stm32cubef0)
INTERACTIVE                     := $(shell [ -t 0 ] && echo 1)

ifdef INTERACTIVE
DOCKER_FLAGS                    := -it
else
DOCKER_FLAGS                    :=
endif

.PHONY: $(MAKECMDGOALS) check cmake make clean

# Always go to the targets check, cmake and make.
# "check" checks if the tf docker dev environment is installed and calls it
# "cmake" calls cmake and generates the Makefile
# "make" calls the Makefile generated by cmake with the CMD given to this Makefile
all $(MAKECMDGOALS): check cmake make

check:
# First we check if the temporary file is there and delete it.
# It might still be there if a PC crashed mid build or similar
	@if [ -f "$(DOCKER_LOCK_FILE)" ]; then \
		rm $(DOCKER_LOCK_FILE); \
	fi
# Then we check if bricklib2 is symlinked correctly
	@if [ ! -d "$(ROOT_DIR)/src/bricklib2" ]; then \
		echo "Could not find bricklib2. Please symlink bricklib2 into src/ folder."; \
		exit 1; \
	fi
# Then we check if stm32cubef0 is installed in bricklib2
	@if [ ! -d "$(STM32CUBEF0_PATH)" ]; then \
		echo "Could not find stm32cubef0 in bricklib2. Please download stm32cubef0 from st and symlink it into the bricklib2/ folder."; \
		exit 1; \
	fi
# Then we check if docker is available and if the build_environment_c container
# is available. If both is the case, we start the docker container, call this
# Makefile in the docker container and write a temporary file
	@if command -v docker >/dev/null 2>&1 ; then \
		if [ $$(docker images -q tinkerforge/build_environment_c) ]; then \
			echo "Using docker image to build."; \
			docker run $(DOCKER_FLAGS) \
			-v $(ROOT_DIR)/../:/$(ROOT_DIR)/../ -u $$(id -u):$$(id -g) \
			-v $(BRICKLIB2_PATH)/:$(BRICKLIB2_PATH)/: -u $$(id -u):$$(id -g) \
			-v $(STM32CUBEF0_PATH)/:$(STM32CUBEF0_PATH)/: -u $$(id -u):$$(id -g) \
			tinkerforge/build_environment_c /bin/bash \
			-c "cd $(ROOT_DIR) ; make $(MAKECMDGOALS)" && \
			touch $(DOCKER_LOCK_FILE); \
		else \
			echo "No docker image found, using local toolchain."; \
		fi \
	fi

cmake:
# If there is the temporary file, we are currently outside of the docker container
# and the docker container is running. In this case we do nothing. Otherwise
# we generate a Makefile with cmake

# Because of a bug in cmake we have to call it two times... For the second
# execution the toolchain files are already defined
	@if [ ! -f $(DOCKER_LOCK_FILE) ]; then \
		if [ ! -f "build/Makefile" ]; then \
			mkdir -p build; \
			cmake -E chdir build/ cmake -DCMAKE_TOOLCHAIN_FILE=$(BRICKLIB2_PATH)/cmake/toolchains/arm-none-eabi.cmake ../; \
			cmake -E chdir build/ cmake ../; \
		fi \
	fi

make:
# If there is the temporary file, we are currently outside of the docker container
# and the docker container is running. In this case we delete the temporary file,
# since this is the last target run outside of the docker container.
# Otherwise we call the Makefile generated by cmake.
	@if [ ! -f $(DOCKER_LOCK_FILE) ]; then \
		cd build/; \
		make $(MAKECMDGOALS); \
	else \
		rm $(DOCKER_LOCK_FILE); \
	fi

clean:
# In case of a clean we just remove the whole build/ directory. Doesn't matter if
# if we are inside or outside of the docker container here.
	@rm -rf build/