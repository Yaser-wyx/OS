AUX_SOURCE_DIRECTORY(kernel DIR_LIB_KERNEL)
aux_source_directory(user DIR_LIB_USER)
aux_source_directory(. DIR_LIB)
ADD_LIBRARY(ALL_LIB ${DIR_LIB_KERNEL} ${DIR_LIB_USER} ${DIR_LIB})