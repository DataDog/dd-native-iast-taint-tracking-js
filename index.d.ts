declare module 'datadog-iast-taint-tracking' {

    export class TaintedUtils {
        createTransaction(transactionId: string): string;
        newTaintedString(transactionId: string, original: string, paramName: string, type: string): string;
        isTainted(transactionId: string, ...args: string[]): boolean;
        getRanges(transactionId: string, original: string): NativeTaintedRange[];
        removeTransaction(transactionId: string): void;
        concat(transactionId: string, result: string, op1: string, op2: string): string;
        substring(transactionId: string, subject: string, result: string, start: number, end: number): string;
    }
}
