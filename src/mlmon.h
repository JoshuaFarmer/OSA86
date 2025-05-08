#ifndef MLMON_H
#define MLMON_H

void mlmon(char * filename)
{
        uint8_t mem[8192];
        memset(mem,0,sizeof(mem));
        if (Exists(filename)!=0)
        {
                FILE fp = fgetf(filename,current_path_idx);
                fseek(&fp,0,SEEK_END);
                int len = ftell(&fp);
                ReadF(filename, mem, len);
        }

        char input = 0;
        int x=0;
        int y=0;
        char buff[3];
        int page=0;
        while (input != 'q')
        {
                int pos=x+(y*16);
                clearScreen(TTY_COL);
                printf("Page: "); PrintByte(page); putc('\n');
                for (int i = 1; i <= 128; ++i)
                {
                        PrintByte(mem[(i-1)+(128*page)]); putc(' ');
                        (i % 8) == 0 ? putc('\n') : 0;
                }
                update_cursor(x*3,y+1);
                switch (input)
                {
                        case 's':
                                if (Exists(filename)==0)
                                        CreateF(filename);
                                WriteF(filename,mem,sizeof(mem));
                                break;
                        case 'H':
                                if (page)
                                        --page;
                                break;
                        case 'L':
                                if (page < 63)
                                        ++page;
                                break;
                        case 'h':
                                if (x)
                                        --x;
                                break;
                        case 'l':
                                if (x < 15)
                                        ++x;
                                break;
                        case 'j':
                                if (y < 15)
                                        ++y;
                                break;
                        case 'k':
                                if (y)
                                        --y;
                                break;
                        case '\n':
                                getsf(buff,3,x*3,y+1,'\n');
                                mem[pos+(128*page)]=strhex(buff);
                                if (x < 15)
                                        ++x;
                                else if (y < 15)
                                {
                                        ++y;
                                        x=0;
                                }
                                else
                                {
                                        y=0;
                                        x=0;
                                        ++page;
                                }
                                break;
                }
                update_cursor(x*3+TTY_XS,y+1+TTY_YS);
                input = getch();
        }
        clearScreen(TTY_COL);
}

#endif