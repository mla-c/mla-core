import './HomePage.css'
import LogViewer from "../components/Log/LogViewer";

export default function HomePage() {

    return (<div className='home-layout'>
        <div className="home-layout__info">
            <h2>Information</h2>
            <div className="pl-container-small">
                <div className="pl-row">
                    <div className="pl-col-2">Name:</div>
                    <div className="pl-col-3">???</div>
                </div>
                <div className="pl-row">
                    <div className="pl-col-2">IP:</div>
                    <div className="pl-col-3">???</div>
                </div>
                <div className="pl-row">
                    <div className="pl-col-2">Mac:</div>
                    <div className="pl-col-3">???</div>
                </div>
                <div className="pl-row">
                    <div className="pl-col-2">SpiffsSize:</div>
                    <div className="pl-col-3">???</div>
                </div>
                <div className="pl-row">
                    <div className="pl-col-2">FlashSize:</div>
                    <div className="pl-col-3">???</div>
                </div>
            </div>
        </div>
        <div className="home-layout__logs">
            <LogViewer/>
        </div>
    </div>)
}
