package org.sipfoundry.commons.timeout;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

public class SipxExecutor {
    public static Result execute(Timeout timeout, int seconds) throws TimeoutException, ExecutionException, InterruptedException{
        ExecutorService executor = Executors.newCachedThreadPool();
        SipxCallable task = new SipxCallable(timeout);
        Future<Object> future = executor.submit(task);
        try {
           return (Result)future.get(seconds, TimeUnit.SECONDS);
        } finally {
           future.cancel(true);
        }
    }
}
