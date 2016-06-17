function [t, Y] = sunnyd(f, T, x0, p)
    %SUNNYD Convenient Matlab interface to the SUNDIALS ODE solvers

    if nargin < 4
        p = []
    end

    if strcmp(class(f), 'function_handle')
        g = recompile();
        [t, Y] = g(f, T, x0, p);
    elseif strcmp(class(f), 'char')
        if exist(f, 'file') == 2
            g = recompile(f);
            [t, Y] = g([], T, x0, p);
        else
            warning('Compilation unit "%s" does not exist', f);
        end
    else
        warning('Usage: [t, Y] = sunnyd( [ filename | function_handle ], T, x0 )')
    end
end

function t = stat(cfile, mfile)
    t = 1;
    if exist(mfile, 'file') == 3
        sc = dir(cfile);
        sm = dir(mfile);
        dc = datevec(sc.date, 'dd-mmm-yyyy HH:MM:ss');
        dm = datevec(sm.date, 'dd-mmm-yyyy HH:MM:ss');
        if etime(dc,dm) < 0
            t = 0;
        end
    end
end

function [f] = recompile(cfile)
    arch = computer('arch');
    path = fileparts(mfilename('fullpath'));

    bits = 32;
    if arch(end) == '4'
        bits = 64; 
    end

    w = '';
    if strfind(arch, 'win')
        w = 'w';
    end
    
    if nargin == 0
        cfile = sprintf('%s/sunnydc.c', path);
    end

    [base,name,ext] = fileparts(cfile);
    cwd = pwd();
    if strcmp(base,'')
        base = '.';
    end
    if stat(cfile, sprintf('%s/%s.mex%s%d', base, name, w, bits))
        chdir(base);
        string = sprintf('mex -I%s/include/Sundials-2.5.0 -I%s/include -DLL %s "%s/lib/%s/sundials_cvode.lib" "%s/lib/%s/sundials_nvecserial.lib"',path,path,cfile,path,arch,path,arch);
        eval(string);
        chdir(cwd);
    end
    f = str2func(name);
end