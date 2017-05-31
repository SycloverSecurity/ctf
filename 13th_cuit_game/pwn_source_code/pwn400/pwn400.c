#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/*
Life Crowdfunding
2017.4.30 by:Ox9A82

Use-After-Free ——> Fastbin Attack
Allocate Fastbin on the bss_name
overwrite bss_name_ptr
leak _IO_FILE get libc_base

leak heap base

600 bytes chunk UAF
fopen alloc _IO_FILE_plus in the UAF chunk
overwrite _IO_FILE_plus IO_jump_t
struct
{
    int number;
    char* Static_str;
    int junk;
}
24 byte

*/


char bss_name[24]="1";

void *bss_name_ptr=1;

char static_string[]="If a man is willing to sacrifice his life for the interests of his country, he will never mind things concerning his personal fortunes and misfortunes.\nMan I'm gonna donate all my seconds to our Holy Toad Empyre!";

int bss_count=1;

void *bss_chunk_ptr=1;

FILE *file_handle=1;

bss_advise_count=1;

void clean(void)
{
    bss_name[0]=0;
    bss_name_ptr=0;
    bss_count=0;
    bss_chunk_ptr=0;
    file_handle=0;
    bss_advise_count=0;
    return;
}

void init(void)
{
    alarm(120);
    setbuf(stdin,0);
    setbuf(stdout,0);
    clean();
    puts("Welcome to life Crowdfunding~~");
}


void welcome(void)
{
    puts("\n\n==============================");
    puts("1.Create a Crowdfunding");//choice 1
    puts("2.Edit my Crowdfunding");//choice 2
    puts("3.Delete my Crowdfunding");//choice 3
    puts("4.Show my Crowdfunding");//choice 4
    puts("5.Submit");//choice 5
    puts("==============================");
    puts("6.SaveAdvise");//choice 6
    puts("7.exit");//choice 7
    puts("==============================");
}

int GetContent(char *ptr,int number)
{
    char temp=0;
    int i=0;
    for(i=0;i<number;i++)
    {
        read(0,&temp,1);
        if(temp!='\n')
        {
            ptr[i]=temp;
        }
        else
        {
            ptr[i]='\x00';
            return;
        }
    }
    ptr[i-1]='\x00';
    return i;
}

int GetNumber(void)
{
    char num_buf[16]="";
    int number=0;
    GetContent(num_buf,8);
    number=atoi(num_buf);
    if(number>=0)
    {
        return number;
    }
    else
    {
        return 0;
    }
    
}


void Create(void)
{
    char stack_buf[210];
    int length=0;
    void *ptr=0;
    void *ptr2=0;

        if(!bss_count)
        {
            bss_count=1;
            ptr=calloc(24,1);
            if(!ptr)
            {
                exit(0);
            }
            if(!bss_name_ptr)
            {
                puts("well,give me your name pls.");
                GetContent(stack_buf,200);
                length=strlen(stack_buf);
                if(length<=24)
                {
                    memcpy(bss_name,stack_buf,20);
                    bss_name_ptr=&bss_name;
                }
                else
                {
                    bss_name_ptr=strdup(stack_buf);
                }
                if(!bss_name_ptr)
                {
                    exit(0);
                }
            }
            puts("How many seconds would you want to Crowdfund?");
            *(int *)ptr=GetNumber();
            printf("+%ds!\n",*(int *)ptr);
            ptr2=(int *)((long long int)ptr+8);
            *(int *)ptr2=(long long int)&static_string;
            ptr2=(int *)((long long int)ptr+16);
            *(int *)ptr2=0x0;
            bss_chunk_ptr=ptr;
        }
        else
        {
           puts("You can only post one Crowdfunding");
        }

}

void Edit(void)
{
    int number=0;

    if(!bss_chunk_ptr)
    {
        puts("No Crowdfunding to edit!");
        return;
    }
    
    if(!bss_count)
    {
        puts("No Crowdfunding to edit!");
        return;
    }
    puts("inputs seconds:");
    number=GetNumber();
    *(int *)bss_chunk_ptr=number;
    printf("Ok,the Crowdfunding is +%ds now!",number);
    return;
}

void Delete(void)
{
    if(!bss_chunk_ptr)
    {
        puts("No Crowdfunding to delete!");
        return;
    }
    
    if(!bss_count)
    {
        puts("No Crowdfunding to delete!");
        return;
    }


    free(bss_chunk_ptr);
    puts("OK,the Crowdfunding is deleted!");
    return;
}

void Show(void)
{
    bss_name_ptr=0;
    printf("Aha We have already have +%d seconds",bss_count);
}


void Submit(void)
{
    char buf[40]="";
    char *email_ptr=0;
    char *message_ptr=0;
    char flag;
    if(!bss_count)
    {
        puts("No frame to submit!");
        return;
    }
    puts("Are you sure submit this post?(Y/N)");
    read(0,&flag,1);
    if(flag!='Y')
    {
        return;
    }
   
    puts("Pls give me your e-mail address");
    GetContent(buf,40);
    email_ptr=strdup(buf);
    if(!email_ptr)
    {
        exit(0);
    }
    puts("OK,e-mail has already posted\nThe last step is do you want to leave some message?");
    GetContent(buf,40);
    message_ptr=strdup(buf);
    return;
}


void SaveAdvise(void)
{
    if(bss_count)
    {
        if(bss_advise_count<2)
      {
        bss_advise_count++;
        void *advise_ptr=0;
        void *title_ptr=0;
        int size=0;
        puts("Pls input advise size:");
        size=GetNumber();
        if(size>32&&size<1024)
        {

        }
        else
        {
            return;
        }
        puts("Pls input tiltle");
        title_ptr=malloc(40);
        advise_ptr=malloc(size);
        if(!file_handle)
        {
            file_handle=fopen("./database","a+");
        }
        GetContent(title_ptr,48);
        puts("Pls input your advise");
        GetContent(advise_ptr,size-0x10);
        printf("OK!(Advise allocate on 0x%x)",advise_ptr);
        fwrite(advise_ptr,1,size-0x10,file_handle);
        free(advise_ptr);
        return;
      }
      else
      {
        puts("Sorry,You have already leave advise!");
        return;
      }
    }
    else
    {
        puts("If you want to leave advise,Pls create Crowdfunding first");
        return;
    }

}


void JumpTo(int choice)
{
    void *ptr=0;
    switch (choice)
    {
        case 1:
            Create();
            break;
        case 2:
            Edit();
            break;
        case 3:
            Delete();
            break;
        case 4:
            Show();
            break;
        case 5:
            Submit();
            break;
        case 6:
            SaveAdvise();
            break;
        case 7:
            exit(0);
            break;
        default:
            if(bss_name_ptr==0)
            {
                puts("Your inputs is wrong~");
            }
            else
            {
                printf("Dear %s,Your inputs is wrong~\n",(char *)bss_name_ptr);
            }
           
    }
    return;

}

int main(void)
{
    int choice=0;
    init();
    while(1)
    {
        welcome();
        choice=GetNumber();
        JumpTo(choice);
    }
   return 0;
}
