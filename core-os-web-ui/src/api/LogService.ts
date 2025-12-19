import {ApiService} from "./ApiService";

export enum LogLevel {
    VERBOSE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
}

export interface LogLevelRequest {
    level: LogLevel;
}

export interface LogEntry {
    logid: number;
    level: LogLevel;
    message: string;
    context1: string;
}

export interface LogMessages {
    entries: LogEntry[];
}


export class LogService {

    public static setLoglevel(request: LogLevelRequest): Promise<void> {
        return ApiService.call("log/setLoglevel", request);
    }

    public static getLoglevel(): Promise<LogLevelRequest> {
        return ApiService.call<LogLevelRequest>("log/getLoglevel");
    }

    public static getLogMessages(): Promise<LogMessages> {
        return ApiService.call<LogMessages>("log/getMessages");
    }


}