#include "cubex360/state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATE_HEADER_SIZE 20u
#define STATE_CPU_SIZE 160u
static const uint8_t state_magic[8] = {'C','U','B','E','X','S','1',0};

static void put32(uint8_t *p, uint32_t v) {
    p[0]=(uint8_t)(v>>24); p[1]=(uint8_t)(v>>16); p[2]=(uint8_t)(v>>8); p[3]=(uint8_t)v;
}
static uint32_t get32(const uint8_t *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|(uint32_t)p[3];
}
static void put64(uint8_t *p, uint64_t v) {
    put32(p,(uint32_t)(v>>32)); put32(p+4,(uint32_t)v);
}
static uint64_t get64(const uint8_t *p) {
    return ((uint64_t)get32(p)<<32)|get32(p+4);
}
static void encode_cpu(uint8_t out[STATE_CPU_SIZE], const cubex_cpu_state_t *c) {
    unsigned i; size_t o=0;
    for(i=0;i<32;i++,o+=4) put32(out+o,c->gpr[i]);
    put32(out+o,c->pc); o+=4; put32(out+o,c->lr); o+=4;
    put32(out+o,c->ctr); o+=4; put32(out+o,c->cr); o+=4;
    put32(out+o,c->xer); o+=4; put32(out+o,c->msr); o+=4;
    put64(out+o,c->retired_instructions);
}
static void decode_cpu(cubex_cpu_state_t *c, const uint8_t in[STATE_CPU_SIZE]) {
    unsigned i; size_t o=0;
    for(i=0;i<32;i++,o+=4) c->gpr[i]=get32(in+o);
    c->pc=get32(in+o); o+=4; c->lr=get32(in+o); o+=4;
    c->ctr=get32(in+o); o+=4; c->cr=get32(in+o); o+=4;
    c->xer=get32(in+o); o+=4; c->msr=get32(in+o); o+=4;
    c->retired_instructions=get64(in+o);
}
static uint32_t crc32_update(uint32_t crc, const uint8_t *p, size_t n) {
    size_t i; unsigned b;
    for(i=0;i<n;i++) {
        crc ^= p[i];
        for(b=0;b<8;b++) crc=(crc>>1)^((crc&1u)?0xedb88320u:0u);
    }
    return crc;
}
static int write_all(FILE *f, const void *p, size_t n) {
    return fwrite(p,1,n,f)==n ? 0 : -1;
}
static int read_all(FILE *f, void *p, size_t n) {
    return fread(p,1,n,f)==n ? 0 : -1;
}
int cubex_state_save(const char *path, const cubex_cpu_state_t *cpu, const cubex_memory_t *mem) {
    uint8_t header[STATE_HEADER_SIZE], cpu_blob[STATE_CPU_SIZE];
    char *tmp; size_t path_len; FILE *f; uint32_t crc=0xffffffffu; int rc=-1;
    if(!path||!cpu||!mem||!mem->main_ram||mem->main_ram_size!=CUBEX360_MAIN_RAM_SIZE) return -1;
    path_len=strlen(path); if(path_len==0||path_len>4000) return -1;
    tmp=(char*)malloc(path_len+5); if(!tmp) return -1;
    memcpy(tmp,path,path_len); memcpy(tmp+path_len,".tmp",5);
    encode_cpu(cpu_blob,cpu);
    crc=crc32_update(crc,cpu_blob,sizeof(cpu_blob));
    crc=crc32_update(crc,mem->main_ram,mem->main_ram_size)^0xffffffffu;
    memcpy(header,state_magic,8); put32(header+8,1u); put32(header+12,(uint32_t)mem->main_ram_size); put32(header+16,crc);
    f=fopen(tmp,"wb");
    if(f) {
        if(write_all(f,header,sizeof(header))==0 &&
           write_all(f,cpu_blob,sizeof(cpu_blob))==0 &&
           write_all(f,mem->main_ram,mem->main_ram_size)==0 &&
           fflush(f)==0) {
            if(fclose(f)==0) { f=NULL; if(rename(tmp,path)==0) rc=0; }
        }
        if(f) fclose(f);
        if(rc!=0) remove(tmp);
    }
    free(tmp); return rc;
}
int cubex_state_load(const char *path, cubex_cpu_state_t *cpu, cubex_memory_t *mem) {
    uint8_t header[STATE_HEADER_SIZE], cpu_blob[STATE_CPU_SIZE]; uint8_t *ram;
    FILE *f; uint32_t crc, expected; int extra, rc=-1; cubex_cpu_state_t candidate;
    if(!path||!cpu||!mem||!mem->main_ram||mem->main_ram_size!=CUBEX360_MAIN_RAM_SIZE) return -1;
    f=fopen(path,"rb"); if(!f) return -1;
    if(read_all(f,header,sizeof(header)) || memcmp(header,state_magic,8)!=0 ||
       get32(header+8)!=1u || get32(header+12)!=(uint32_t)mem->main_ram_size ||
       read_all(f,cpu_blob,sizeof(cpu_blob))) { fclose(f); return -1; }
    ram=(uint8_t*)malloc(mem->main_ram_size); if(!ram) { fclose(f); return -1; }
    if(read_all(f,ram,mem->main_ram_size)!=0) { free(ram); fclose(f); return -1; }
    extra=fgetc(f); if(extra!=EOF || ferror(f)) { free(ram); fclose(f); return -1; }
    fclose(f);
    crc=crc32_update(0xffffffffu,cpu_blob,sizeof(cpu_blob));
    crc=crc32_update(crc,ram,mem->main_ram_size)^0xffffffffu;
    expected=get32(header+16);
    if(crc==expected) {
        candidate=*cpu; decode_cpu(&candidate,cpu_blob);
        memcpy(mem->main_ram,ram,mem->main_ram_size); *cpu=candidate; rc=0;
    }
    free(ram); return rc;
}
