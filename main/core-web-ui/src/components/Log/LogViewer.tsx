import "./LogViewer.css"

import {useCallback, useMemo} from "preact/hooks";
import {LogEntry, LogLevel} from "../../api/LogService";
import {useLogLevel, useLogs} from "../../hooks/useLogService";

type LogViewerProps = {
    logFilter?: (log: LogEntry) => boolean
}

export default function LogViewer({logFilter}: LogViewerProps) {

    const [logLevel, setLogLevel, isLoadingLogLevel] = useLogLevel();
    const [logs] = useLogs();

    const logLevelChanged = useCallback(async (e: any) => {
        await setLogLevel(parseInt(e.target.value));
    }, [setLogLevel]);

    const getColorByLevel = useCallback((level: number | undefined) => {

        if (level === LogLevel.ERROR) {
            return 'text-red';
        }

        if (level === LogLevel.WARN) {
            return 'text-yellow';
        }

        if (level === LogLevel.DEBUG) {
            return 'text-blue';
        }

        if (level === LogLevel.VERBOSE) {
            return 'text-gray';
        }

        if (level === LogLevel.ERROR + 1) {
            return 'text-orange';
        }

        return '';

    }, []);

    const finalLogFilter = useMemo(() => {

        if (!logFilter)
            return () => true;

        return logFilter;

    }, [logFilter]);

    return <>
        <div className="d-flex">
            <h4>Logs</h4>
            {!isLoadingLogLevel && <select className="logviewer__loglevel" value={logLevel} onChange={logLevelChanged}>
                <option value={LogLevel.ERROR}>Error</option>
                <option value={LogLevel.WARN}>Warning</option>
                <option value={LogLevel.INFO}>Info</option>
                <option value={LogLevel.DEBUG}>Debug</option>
                <option value={LogLevel.VERBOSE}>Verbose</option>
            </select>}
        </div>

        <div className="pl-container-large">
            <div className="pl-row">
                <div className="pl-col-2 text-bold">Level</div>
                <div className="pl-col-10 text-bold">Message</div>
            </div>
            {logs?.filter(finalLogFilter).map((x, index) => {
                return <div className="pl-row" key={index}>
                    <div className={'pl-col-2 ' + getColorByLevel(x.level)}>{x.level}</div>
                    <div className="pl-col-10">{x.message}</div>
                </div>
            })}

        </div>
    </>

}