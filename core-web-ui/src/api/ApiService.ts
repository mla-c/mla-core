export class ApiService {
    static BASE_URL = '../rpc/';
    //static BASE_URL = 'http://localhost:8081/rpc/';

    public static async call<T>(method: string, params?: any): Promise<T> {
        const response = await fetch(ApiService.BASE_URL + method, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: params ? JSON.stringify(params) : undefined
        });

        if (!response.ok) {
            throw new Error(`API call failed with status ${response.status}`);
        }

        const data = await response.json();
        return data as T;
    }
}

