gcc -c actividad1.c
gcc -c actividad2.c
ar cr liblab2.a actividad1.o actividad2.o 
mv liblab2.a ../lib
cd..
cd..
gcc -c main.c -I liblab2/include -I cJSONlib //indico ubicacion de los headers
gcc main.o -L liblab2/lib -l:liblab2.a -L cJSONlib/lib -l:libjson.a -o main indico ubicaciones de los .a

---------------------------------
gcc -c -fPIC plugin.c -I ../cJSONlib -I . 
gcc -shared -fPIC -o libplugin.so plugin.o
mv libplugin.so lib

gcc -c main.c -I liblab2/include -I cJSONlib -I plugin
gcc main.o -L liblab2/lib -l:liblab2.a -L cJSONlib/lib -l:libjson.a -L plugin/lib -l:libplugin.so -o main
cd 
cd plugin/lib/
sudo cp libplugin.so /usr/local/lib //para agregar la lib a directorio del sistema que tiene permisos
sudo ldconfig //para que linux recompute las lib instaladas




MemTotal:       15733056 kB
MemFree:         1830116 kB
MemAvailable:   10986072 kB
Buffers:          557944 kB
Cached:          8765908 kB
SwapCached:            0 kB
Active:          2856044 kB
Inactive:        9458452 kB
Active(anon):       9420 kB
Inactive(anon):  3147072 kB
Active(file):    2846624 kB
Inactive(file):  6311380 kB
Unevictable:          64 kB
Mlocked:              64 kB
SwapTotal:      17563376 kB
SwapFree:       17563376 kB
Dirty:               304 kB
Writeback:             0 kB
AnonPages:       2990732 kB
Mapped:          1153308 kB
Shmem:            177884 kB
KReclaimable:     332364 kB
Slab:             525848 kB
SReclaimable:     332364 kB
SUnreclaim:       193484 kB
KernelStack:       20160 kB
PageTables:        52820 kB
NFS_Unstable:          0 kB
Bounce:                0 kB
WritebackTmp:          0 kB
CommitLimit:    25429904 kB
Committed_AS:   14018504 kB
VmallocTotal:   34359738367 kB
VmallocUsed:       64208 kB
VmallocChunk:          0 kB
Percpu:            23104 kB
HardwareCorrupted:     0 kB
AnonHugePages:         0 kB
ShmemHugePages:        0 kB
ShmemPmdMapped:        0 kB
FileHugePages:         0 kB
FilePmdMapped:         0 kB
HugePages_Total:       0
HugePages_Free:        0
HugePages_Rsvd:        0
HugePages_Surp:        0
Hugepagesize:       2048 kB
Hugetlb:               0 kB
DirectMap4k:     1272368 kB
DirectMap2M:    13799424 kB
DirectMap1G:     2097152 kB



processor	: 0
vendor_id	: AuthenticAMD
cpu family	: 23
model		: 96
model name	: AMD Ryzen 7 4700U with Radeon Graphics
stepping	: 1
microcode	: 0x8600106
cpu MHz		: 2000.000
cache size	: 512 KB
physical id	: 0
siblings	: 8
core id		: 0
cpu cores	: 8
apicid		: 0
initial apicid	: 0
fpu		: yes
fpu_exception	: yes
cpuid level	: 16
wp		: yes
flags		: fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 ht syscall nx mmxext fxsr_opt pdpe1gb rdtscp lm constant_tsc rep_good nopl nonstop_tsc cpuid extd_apicid aperfmperf rapl pni pclmulqdq monitor ssse3 fma cx16 sse4_1 sse4_2 movbe popcnt aes xsave avx f16c rdrand lahf_lm cmp_legacy svm extapic cr8_legacy abm sse4a misalignsse 3dnowprefetch osvw ibs skinit wdt tce topoext perfctr_core perfctr_nb bpext perfctr_llc mwaitx cpb cat_l3 cdp_l3 hw_pstate ssbd mba ibrs ibpb stibp vmmcall fsgsbase bmi1 avx2 smep bmi2 cqm rdt_a rdseed adx smap clflushopt clwb sha_ni xsaveopt xsavec xgetbv1 xsaves cqm_llc cqm_occup_llc cqm_mbm_total cqm_mbm_local clzero irperf xsaveerptr rdpru wbnoinvd cppc arat npt lbrv svm_lock nrip_save tsc_scale vmcb_clean flushbyasid decodeassists pausefilter pfthreshold avic v_vmsave_vmload vgif v_spec_ctrl umip rdpid overflow_recov succor smca
bugs		: sysret_ss_attrs spectre_v1 spectre_v2 spec_store_bypass retbleed
bogomips	: 3992.71
TLB size	: 3072 4K pages
clflush size	: 64
cache_alignment	: 64
address sizes	: 48 bits physical, 48 bits virtual
power management: ts ttp tm hwpstate cpb eff_freq_ro [13] [14]

Linux version 5.15.0-47-generic (buildd@lcy02-amd64-060) (gcc (Ubuntu 11.2.0-19ubuntu1) 11.2.0, GNU ld (GNU Binutils for Ubuntu) 2.38) #51-Ubuntu SMP Thu Aug 11 07:51:15 UTC 2022

gcc -c actividad1.c -o ../obj/actividad1.o
gcc -c actividad2.c -o ../obj/actividad2.o

ar cr ../lib/liblab2.a actividad1.o actividad2.o

gcc -c cJSON.c -I ../include -o ../obj/cJSON.o
gcc -c -fPIC plugin.c -I ../include -o ../obj/plugin.o  
gcc -shared -fPIC -o ../lib/libfile.so plugin.o

gcc -c main.c -I /include -o ../obj/main.o

gcc main.o -L liblab2/lib -l:liblab2.a -L cJSONlib/lib -l:libjson.a -L plugin/lib -l:libplugin.so -o main

gcc main.o -L ../lib -l:liblab2.a -l:libjson.a -l:libplugin.so -o ../bin/main