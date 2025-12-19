import {useCallback, useEffect, useState} from "preact/hooks";
import {LogEntry, LogLevel, LogService} from "../api/LogService";
import useAsyncEffect from "./useCancellableEffect";
import {useLoadingState} from "./useLoadingState";


export function useLogs() {

    const [logs, setLogs] = useState<LogEntry[]>([])
    const [isLoading, startLoading, stopLoading] = useLoadingState();

    const loadLogs = useCallback(async () => {

        startLoading();
        try {
            const request = await LogService.getLogMessages();
            return request.entries;
        } finally {
            stopLoading();
        }

    }, [startLoading]);

    const mergeLogs = useCallback((newLogs: LogEntry[]) => {

        setLogs((prev) => {

            // If the last log of the previous logs is not the first log -1 of the new logs, we have a gap
            // So we need to put there a separator log with level ERROR + 1
            if (prev.length > 0 && newLogs.length > 0) {
                const lastPrevLog = prev[prev.length - 1];
                const firstNewLog = newLogs[0];
                if (firstNewLog.logid !== lastPrevLog.logid + 1) {
                    const separatorLog: LogEntry = {
                        logid: lastPrevLog.logid + 1,
                        level: LogLevel.ERROR + 1,
                        message: '--- Log gap detected. Missing Frames ' + (lastPrevLog.logid - firstNewLog.logid) + ' ---',
                        context1: ''
                    };
                    return [...prev, separatorLog, ...newLogs];
                }
            }

            return [...prev, ...newLogs];

        });

    }, []);

    // Update the logs every 5 seconds
    useEffect(() => {

        const interval = setInterval(async () => {

            const newLogs = await loadLogs();
            mergeLogs(newLogs);

        }, 5000);

        return () => clearInterval(interval);
    }, [loadLogs, mergeLogs]);

    return [logs, isLoading] as const;
}

export function useLogLevel() {

    const [logLevel, setLogLevel] = useState<LogLevel>(undefined);
    const [isLoading, startLoading, stopLoading] = useLoadingState();

    const loadLogLevel = useCallback(async () => {

        startLoading();
        try {
            const request = await LogService.getLoglevel();
            return request.level;
        } finally {
            stopLoading();
        }

    }, [startLoading, stopLoading]);

    useAsyncEffect(loadLogLevel, setLogLevel);

    const updateLogLevel = useCallback(async (newLogLevel: LogLevel) => {

        setLogLevel(newLogLevel)
        startLoading();
        try {
            await LogService.setLoglevel({level: newLogLevel});
        } finally {
            stopLoading()
        }

    }, [startLoading, stopLoading]);

    return [logLevel, updateLogLevel, isLoading] as const;

}