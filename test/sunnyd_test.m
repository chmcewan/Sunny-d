function [] = sunnyd_test()
    a      = 103;
    b      = 1;
    
    num_data    = 200;
    tmax        = 2.0;
    x0          = [2,0];
    T           = 0:0.01:tmax;
    
    clf;
    subplot(3, 1, 1);
    [t,Y] = ode15s(@dx, T, x0, [], [a, b]);
    render(t,Y, 'ode15s');
    
    subplot(3, 1, 2);
    [t,Y] = sunnyd(@dx, T, x0, [a, b]);
    render(t,Y, 'sunnyd (Matlab)');

    subplot(3, 1, 3);
    [t,Y] = sunnyd('sunnyd_test_c.c', T, x0, [a, b]);
    render(t,Y, 'sunnyd (C)');    
    
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
