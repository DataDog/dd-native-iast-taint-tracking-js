declare module 'datadog-iast-taint-tracking' {

    export interface NativeInputInfo {
        parameterName: string;
        parameterValue: string;
        type: string;
        readonly ref?: string;
    }

    export interface NativeTaintedRange {
        start: number;
        end: number;
        iinfo: NativeInputInfo;
        readonly ref?: string;
    }

    export class TaintedUtils {
        createTransaction(transactionId: string): string;
        newTaintedString(transactionId: string, original: string, paramName: string, type: string): string;
        isTainted(transactionId: string, ...args: string[]): boolean;
        getRanges(transactionId: string, original: string): NativeTaintedRange[];
        removeTransaction(transactionId: string): void;
        concat(transactionId: string, op1: string, op2: string): string;
    }
}
