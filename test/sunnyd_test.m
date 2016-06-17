function [] = sunnyd_test()
    a      = 103;
    b      = 1;
    
    num_data    = 200;
    tmax        = 2.0;
    x0          = [2,0];
    T           = 0:0.01:tmax;
    
    clf;
    subplot(3, 1, 1);
    f1 = @() ode15s(@dx, T, x0, [], [a, b]);
    [t,Y] = f1();
    t1 = timeit(f1);
    render(t,Y, sprintf('ode15s (%.6fs)', t1));
    
    subplot(3, 1, 2);
    f2 = @() sunnyd(@dx, T, x0, [a, b]);
    [t,Y] = f2();
    t2 = timeit(f2);
    render(t,Y, sprintf('sunnyd/Matlab (%.1fx faster)', t1/t2));

    subplot(3, 1, 3);
    f3 = @() sunnyd('sunnyd_test_c.c', T, x0, [a, b]);
    [t,Y] = f3();
    t3 = timeit(f3);
    render(t,Y, sprintf('sunnyd/C (%.1fx faster)', t1/t3));    
    
end

function [] = render(t, Y, name)
    hold on;
    plot(t,Y(:,1),'r.-','linewidth',1);
    plot(t,Y(:,2),'g.-','linewidth',1);
    title(name)
    drawnow;
end

function d = dx(t,y,P)
    a = P(1);
    b = P(2);
    d = [y(2); -a * (y(2) * (y(1) * y(1) - b) + y(1)) ];   
end
