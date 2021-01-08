#include<types.h>
#include<mmap.h>

// Helper function to create a new vm_area
struct vm_area* create_vm_area(u64 start_addr, u64 end_addr, u32 flags, u32 mapping_type)
{
	struct vm_area *new_vm_area = alloc_vm_area();
	new_vm_area-> vm_start = start_addr;
	new_vm_area-> vm_end = end_addr;
	new_vm_area-> access_flags = flags;
	new_vm_area->mapping_type = mapping_type;
	return new_vm_area;
}

/**
 * Function will invoked whenever there is page fault. (Lazy allocation)
 * 
 * For valid access. Map the physical page 
 * Return 0
 * 
 * For invalid access, i.e Access which is not matching the vm_area access rights (Writing on ReadOnly pages)
 * return -1. 
 */
int check_length(int len){
 if(len % 0x1000 != 0)
                {len = len + (0x1000 - len % 0x1000);}
return len;
        
}
int print_fail(struct exec_context *current, u64 addr, int length){
	if(!(current))
	{printk("fail 1");
	return 1;
	}
	if(!length)
	{printk("fail 2");
	return 1;
	}
	if((addr + length) > MMAP_AREA_END)
	{printk("fail 3");
	return 1;
	} 
	if(((addr%4096) != 0) && flags == MAP_FIXED)
	{printk("fail 4");
	return 1;
	} 
	return 0;
}
u64 remove_pw(struct exec_context *ctx, u64 addr, int length){
	
	u64 rem = addr+length;
	while (addr < rem){
		u64 pfn;
	u64 *vaddr_base = (u64 *)osmap(ctx->pgd);
    u64 *entry;
	
	entry = vaddr_base + ((addr & PGD_MASK) >> PGD_SHIFT);
    if(!(*entry & 0x1)) { continue; }
    else{ pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
        vaddr_base = (u64 *)osmap(pfn);}   
	
	entry = vaddr_base + ((addr & PUD_MASK) >> PUD_SHIFT);    
	if(!(*entry & 0x1)) {continue;}
	else {pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
     vaddr_base = (u64 *)osmap(pfn);}

	entry = vaddr_base + ((addr & PMD_MASK) >> PMD_SHIFT);
    if(!(*entry & 0x80)){// 
	   if(*entry & 0x1) {
        pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
    	vaddr_base = (u64 *)osmap(pfn);
    	}
		entry = vaddr_base + ((addr & PTE_MASK) >> PTE_SHIFT);
    	if(*entry & 0x1) {
            pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
            os_pfn_free(USER_REG, pfn);
            *entry = *entry & ~(0x1);
            asm volatile ("invlpg (%0);":: "r"(addr): "memory" );
            }
        addr += 0x1000;
	}else{
    pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
    *entry = *entry & ~(0x1); 
    asm volatile ("invlpg (%0);":: "r"(addr) : "memory" );
     addr += 0x10000;
	}            
  }
}
int vm_area_pagefault(struct exec_context *current, u64 addr, int error_code)
{
	int e = error_code;
	int p= (e & 1), w= (e >> 1)& 1, u = (e >> 2)&1;
if (p==1){
	if (w==1){
			struct vm_area *i = current->vm_area->vm_next; 
			while(i){
				if(addr >= i->vm_start && addr <= i->vm_end){
					if(PROT_READ & (i->access_flags) ){
						return -1;
					}
				}
				i = i->vm_next;
			}
		
	}
}
u64 pfn;
u64 *entry;
u64 *vaddr_base = (u64 *)osmap(current->pgd);
entry = vaddr_base + ((addr & PGD_MASK) >> PGD_SHIFT);
u64 ac_flags =( 0x5 | (error_code & 0x2));
		
		
		
		if(!(*entry & 0x1)) {
			pfn = os_pfn_alloc(OS_PT_REG);
			*entry = (pfn << PTE_SHIFT) | ac_flags;
			vaddr_base = osmap(pfn);
			
			
		}else{
			pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
			vaddr_base = (u64 *)osmap(pfn);
			
		}

		entry = vaddr_base + ((addr & PUD_MASK) >> PUD_SHIFT);
		if(!(*entry & 0x1)) {
			pfn = os_pfn_alloc(OS_PT_REG);
			*entry = (pfn << PTE_SHIFT) | ac_flags;
			vaddr_base = osmap(pfn);
			
		}else{
			
			pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
			vaddr_base = (u64 *)osmap(pfn);
		}

		entry = vaddr_base + ((addr & PMD_MASK) >> PMD_SHIFT);
		if(!(*entry & 0x40)){
			if(*entry & 0x1) {
				pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
				vaddr_base = (u64 *)osmap(pfn);
			}else{
				pfn = os_pfn_alloc(OS_PT_REG);
				*entry = (pfn << PTE_SHIFT) | ac_flags;
				vaddr_base = osmap(pfn);
			}

			entry = vaddr_base + ((addr & PTE_MASK) >> PTE_SHIFT);
			pfn = os_pfn_alloc(USER_REG);
			*entry = (pfn << PTE_SHIFT) | ac_flags;
		
		}else{
			pfn = (*entry >> PTE_SHIFT) & 0xFFFFFFFF;
			if(*entry & 0x1){
				*entry &= ~1;
			}
		}
		return 1;
}


/**
 * mmap system call implementation.
 */
long vm_area_map(struct exec_context *current, u64 addr, int length, int prot, int flags)
{ int flg = print_fail(current, addr, length);
if (flg){
	return -1;
}
if(length%4096)
{length = length  - (length%4096)+ 4096  ;
}
if(addr%4096)
{addr = addr  - (addr%4096) + 4096  ;
}
if (current->vm_area){
	if (addr){//
					struct vm_area *temp = current->vm_area->vm_next; 

		if(temp->vm_start < (length + addr)){ //
							struct vm_area *tmp = current->vm_area->vm_next; 

				while((addr < tmp->vm_end || (addr + length) > tmp->vm_next->vm_start) &&  tmp->vm_next){
					if((addr <= tmp->vm_start && (addr + length) <= tmp->vm_end) || (addr >= tmp->vm_start && (addr + length) >= tmp->vm_end)){ //in case the addr already allocated
						if(flags == MAP_FIXED){return -1;}
						return vm_area_map(current,0,length,prot,flags);
					}
					tmp = tmp->vm_next;
				}
				if(!(tmp->access_flags ^ prot) && tmp->vm_end == addr){ //can merge the new node with its previous node
					tmp->vm_end += length;
					if(tmp->vm_next && tmp->vm_end == tmp->vm_next->vm_start && !(tmp->access_flags ^ tmp->vm_next->access_flags)){//can merge with next node
						tmp->vm_end = tmp->vm_next->vm_end;
						struct vm_area *remove_node = tmp->vm_next;
						tmp->vm_next = tmp->vm_next->vm_next;
						dealloc_vm_area(remove_node);
						return tmp->vm_start;
					}
					return tmp->vm_start;
				}
				else if(tmp->vm_next && (addr + length) == tmp->vm_next->vm_start && !(prot ^ tmp->vm_next->access_flags)){//can merge with next node
					tmp->vm_next->vm_start = addr;
					return addr;
				}else{
					if(!(tmp->vm_next)){//
						struct vm_area *new_node = create_vm_area(addr,addr + length,prot,NORMAL_PAGE_MAPPING);
						new_node->vm_next = NULL;
						tmp->vm_next = new_node;
						return addr;
						
						
					}else{
						struct vm_area *new_node = create_vm_area(addr,addr + length,prot,NORMAL_PAGE_MAPPING);
						new_node->vm_next = tmp->vm_next;
						tmp->vm_next = new_node;
						return addr;
					}
				}
		
			
				
			}else{
				if(temp->vm_start != length + addr){//
					struct vm_area *new_node = create_vm_area(addr,addr + length,prot,NORMAL_PAGE_MAPPING);
					new_node->vm_next = current->vm_area->vm_next;
					current->vm_area->vm_next = new_node;
					
					return addr;
					
					
				}else{
					if(tmp->access_flags ^ prot){//
						struct vm_area *newnode = create_vm_area(addr,addr + length,prot,NORMAL_PAGE_MAPPING);
						newnode->vm_next = current->vm_area->vm_next;
						current->vm_area->vm_next = newnode;
						
						return addr;
						
						
					}else{
						current->vm_area->vm_next->vm_start = addr;
						
						return addr;
					}
				}
			}
		}else
		{//
			
			if(current->vm_area->vm_next->vm_start >= length + MMAP_AREA_START + 4096){ //new node to be inserted before head node
				if(current->vm_area->vm_next->vm_start == length + MMAP_AREA_START + 4096){//new node could be merged if protections match
					if(!(current->vm_area->vm_next->access_flags ^ prot)){
						current->vm_area->vm_next->vm_start = MMAP_AREA_START+4096;
						
						return current->vm_area->vm_next->vm_start ;
					}else{
						struct vm_area *new_node = create_vm_area(MMAP_AREA_START+4096,MMAP_AREA_START+4096+length,prot,NORMAL_PAGE_MAPPING);
						new_node->vm_next = current->vm_area->vm_next;
						current->vm_area->vm_next = new_node;
						//printk("%ld ",new_node->vm_start);
						return new_node->vm_start;
					}
				}else{
					struct vm_area *new_node = create_vm_area(MMAP_AREA_START+4096,MMAP_AREA_START+4096+length,prot,NORMAL_PAGE_MAPPING);
					new_node->vm_next = current->vm_area->vm_next;
					current->vm_area->vm_next = new_node;
					//printk("%ld ",new_node->vm_start);
					return new_node->vm_start;
				}
			}else{
				struct vm_area *temp = current->vm_area->vm_next; // Ommiting the dummy head nodes
				while(temp->vm_next && temp->vm_end + length > temp->vm_next->vm_start){
					temp = temp->vm_next;
				}
				if(!(temp->access_flags ^ prot)){ //can merge the new node with its previous node
					long return_addr = temp->vm_end;
					temp->vm_end += length;
					if(temp->vm_next && temp->vm_end == temp->vm_next->vm_start && !(temp->access_flags ^ temp->vm_next->access_flags)){//can merge with next node
						temp->vm_end = temp->vm_next->vm_end;
						struct vm_area *remove_node = temp->vm_next;
						temp->vm_next = temp->vm_next->vm_next;
						dealloc_vm_area(remove_node);
						//printk("%ld ",return_addr);
						return temp->vm_start;
					}
					//printk("%ld ",return_addr);
					return temp->vm_start;
				}
				else if(temp->vm_next && temp->vm_end + length == temp->vm_next->vm_start && !(prot ^ temp->vm_next->access_flags)){//can merge with next node
					temp->vm_next->vm_start = temp->vm_end;
					//printk("%ld ",temp->vm_next->vm_start);
					return temp->vm_next->vm_start;
				}else{
					if(temp->vm_next){//in between two nodes with access flags different from prot
						struct vm_area *new_node = create_vm_area(temp->vm_end,temp->vm_end+length,prot,NORMAL_PAGE_MAPPING);
						new_node->vm_next = temp->vm_next;
						temp->vm_next = new_node;
						//printk("%ld ",new_node->vm_start);
						return new_node->vm_start;
					}else{// end of the list
						struct vm_area *new_node = create_vm_area(temp->vm_end,temp->vm_end+length,prot,NORMAL_PAGE_MAPPING);
						new_node->vm_next = NULL;
						temp->vm_next = new_node;
						//printk("%ld ",new_node->vm_start);
						return new_node->vm_start;
					}
				}
			}
		}
		
	}else
	{
	struct vm_area *nnode = create_vm_area(MMAP_AREA_START,MMAP_AREA_START+4096,0x4,NORMAL_PAGE_MAPPING);
		current->vm_area = nnode;
		if(!(addr)){//
			struct vm_area *new = create_vm_area(current->vm_area->vm_end,current->vm_area->vm_end+length,prot,NORMAL_PAGE_MAPPING);
			new->vm_next = NULL;
			current->vm_area->vm_next = new;
			return new->vm_start;
			
			
		}else{
			struct vm_area *new = create_vm_area(addr,addr + length,prot,NORMAL_PAGE_MAPPING);
			new->vm_next = NULL;
			current->vm_area->vm_next = new;
			return new->vm_start;
		}		
		
	}
	return 0;
	
}


/**
 * munmap system call implemenations
 */
int vm_area_unmap(struct exec_context *current, u64 addr, int length)
{   length = check_length(length);
u64 end = addr + length;

        struct vm_area *curr = current->vm_area->vm_next;
		struct vm_area *last = current->vm_area;
		while (curr && curr->vm_start <= end )
		{	

			if (curr->vm_end <= end){
				if (curr->vm_start >= addr ){
					last->vm_next = curr->vm_next;
                        remove_pw(current, curr->vm_start, curr->vm_end - curr->vm_start);
                        dealloc_vm_area(curr);
                        curr = last->vm_next;
                        continue;
				}else if (curr->vm_start < addr && curr->vm_end >= addr){
					remove_pw(current, addr, curr->vm_end);
                        curr->vm_end = addr;
				}
			}else{
				 if (curr->vm_start >= addr &&  curr->vm_start <= end )
					{ remove_pw(current, curr->vm_start, end);
                        curr->vm_start = end;
				
			   		}
				else if (curr->vm_start < addr )
			   		{struct vm_area *temp = create_vm_area(end, curr->vm_end, curr->access_flags, curr->mapping_type);
                        remove_pw(current, addr, end);
                        curr->vm_end = addr;
                        curr->vm_next = temp;
				
					}
			}
			last = curr;
			curr = curr->vm_next;

		}
		
	return 0;
}


/**
 * make_hugepage system call implemenation
 */
long vm_area_make_hugepage(struct exec_context *current, void *addr, u32 length, u32 prot, u32 force_prot)
{	u64 end = addr + length;

        
	return 0;
}


/**
 * break_system call implemenation
 */
int vm_area_break_hugepage(struct exec_context *current, void *addr, u32 length)
{
	return 0;
}
